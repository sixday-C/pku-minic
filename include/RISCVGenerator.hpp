#pragma once
#include "ir.hpp"
#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>

struct StackLayout {
    int S,R,A,total;
    int raOffset;
    StackLayout() : S(0), R(0), A(0), total(0), raOffset(0) {}
    StackLayout(int s, int r, int a, int t, int ra) 
        : S(s), R(r), A(a), total(t), raOffset(ra) {}
};

StackLayout computeLayout(const Function& func,std::map<std::string, int>& stackMap){
    int S=0,R=0,A=0;
    int maxArgs=0;
    bool hasCall=false;

    //计算 call 的数量来决定 R的大小
    for(const auto& block : func.blocks) {
        for (const auto& inst : block->insts) {
            if (inst->op == OpType::Call) {
                hasCall=true;
                auto callInst = static_cast<CallInst*>(inst.get());
                maxArgs = std::max(maxArgs, (int)callInst->args.size());
            }
        }
    }
    //计算 A 的大小
    A = std::max(0, maxArgs - 8) * 4;
    //计算 R 的大小
    R = hasCall ? 4 : 0;
    //计算 S 的大小         分两部分：参数和 alloc/int32类型的指令
    int offset=A;
    for (const auto& param : func.params) {
    stackMap[param.first] = offset; // 给参数名登记 offset
    offset += 4;                   // 每个参数占 4 字节
    }
    for(const auto& block : func.blocks) {
        for (const auto& inst : block->insts) {
            if (inst->op == OpType::Alloc) {
                auto alloc = static_cast<AllocInst*>(inst.get());
                stackMap[inst->name] = offset;
                offset += alloc->arraySize * 4; // 数组占 size * 4
            } 
            else {
            switch (inst->type) {
                case Type::Int32:
                case Type::Pointer:
                    stackMap[inst->name] = offset;
                    offset += 4;
                    break;
                case Type::Void:
                case Type::Label:
                    break;
            }
        }
        }
    }
    S=offset-A;
    //计算栈上总分配 total 的大小
    int total = (A + S + R + 15) / 16 * 16;
    int raOffset = total - R; // ra 存在 R 区域的最后
    return {S,R,A,total,raOffset};
}
StackLayout currentLayout;

class RISCVGenerator {
private:
    std::stringstream ss;
    std::string currentFuncLabel;
    bool isFirstBlockInCurrentFunc = false;

    std::string getAsmBlockLabel(const std::string& irBlockName) const {
        std::string block = irBlockName;
        if (!block.empty() && block[0] == '%') {
            block = block.substr(1);
        }
        if (currentFuncLabel.empty()) {
            return block;
        }
        return currentFuncLabel + "_" + block;
    }
public:
    std::string generate(const Program& prog) {
    ss.str(""); 
    ss.clear();
    // --- 第一步：处理全局变量（数据段） ---
    if (!prog.globalValues.empty()) {
        ss << "  .data\n"; // 告诉汇编器，接下来的东西放数据段
        for (const auto& val : prog.globalValues) {
            // 晶，这里要把 Value 强转成你定义的 GlobalAlloc
            auto* global = static_cast<GlobalAlloc*>(val.get());
            
            // 去掉名字开头的 '@'
            std::string label = global->name.substr(1);
            
            ss << "  .globl " << label << "\n"; // 声明全局符号
            ss << label << ":\n";               // 变量标签
            if (global->values.empty()) {
            // 如果是 zeroinit，根据 size * 4 填充 0
            ss << "  .zero " << global->size * 4 << "\n";
            } else {
                for (int v : global->values) {
                    ss << "  .word " << v << "\n";
                }
            }
            ss << "\n";
        }
    }
    

    // --- 第二步：处理函数定义（代码段） ---
    for (const auto& func : prog.funcs) {
        if (func->blocks.empty()) { 
            continue; 
        }
        ss << "  .text\n"; // 切换回代码段
        ss << "  .globl " << func->name.substr(1) << "\n";
        visit(*func);
        ss << "\n";
    }
    
    return ss.str();
}

   std::string getValRegFromStack(Value* val, const std::string& tempReg) {
    std::string name=val->name;
    if(name=="0"){
        return "x0";
    }
    if (val->isGlobal()) {
        ss << "  la " << tempReg << ", " << name.substr(1) << "\n";
        ss << "  lw " << tempReg << ", 0(" << tempReg << ")\n";
        return tempReg;
    }
    if(name[0]=='@'||name[0]=='%'){//变量或临时变量
        int offset = getStackOffset(name);
        ss << "  lw " << tempReg << ", " << offset << "(sp)\n";//从栈上加载到临时寄存器
        return tempReg;
    }
    else{
        //立即数
        ss << "  li " << tempReg << ", " << name << "\n";
        return tempReg;
    }
}

    int getStackOffset(const std::string& name) {
        if (stackMap.find(name) != stackMap.end()) {
            return stackMap[name];
        }
        throw std::runtime_error("Variable not found in stack map: " + name);
    }

    std::map<std::string,int> stackMap;
    int stackSize=0;


    void visit(const Function& func) {
        stackMap.clear();
        stackSize=0;
        isFirstBlockInCurrentFunc = true;
        currentLayout = computeLayout(func, stackMap);
        int total=currentLayout.total;
        currentFuncLabel = func.name.substr(1);
        
        ss<<currentFuncLabel << ":\n";
        if (total > 0) {
        if (total <= 2048) {
            ss << "  addi sp, sp, -" << total << "\n";
        } else {
            ss << "  li t0, -" << total << "\n";
            ss << "  add sp, sp, t0\n";
        }
    }
        if (currentLayout.R > 0) {
        ss << "  sw ra, " << currentLayout.raOffset << "(sp)\n";
    }
        for (size_t i = 0; i < func.params.size(); ++i) {
            std::string paramName = func.params[i].first;
            int offset = getStackOffset(paramName);

            if (i < 8) {
            std::string reg = "a" + std::to_string(i);
            ss << "  sw " << reg << ", " << offset << "(sp)\n";
            }
            else {
                int argOffset = currentLayout.total+(i - 8) * 4;
                ss << "  lw t0, " << argOffset << "(sp)\n";
                ss << "  sw t0, " << offset << "(sp)\n";
            }
    }
        for(const auto& block : func.blocks) {
            visit(*block);
        }

    }


    void visit(const BasicBlock& block) {
        bool isEntryBlock = (block.name == "%entry" || block.name == "entry");
        if (!(isFirstBlockInCurrentFunc && isEntryBlock)) {
            ss<<getAsmBlockLabel(block.name)<< ":\n";
        }
        isFirstBlockInCurrentFunc = false;
        for (const auto& inst : block.insts) {
            visit(*inst);
        }
    }
    void visit(const Instruction& inst) {
        if (inst.op == OpType::Ret) {
            visitReturn(static_cast<const ReturnInst&>(inst));
        }
        else if(inst.op==OpType::Alloc){
            visitAlloc(static_cast<const AllocInst&>(inst));
        }
        else if(inst.op==OpType::Store){
            visitStore(static_cast<const StoreInst&>(inst));
        }
        else if(inst.op==OpType::Load){
            visitLoad(static_cast<const LoadInst&>(inst));
        }
        else if(inst.op==OpType::Br){
            visitBranch(static_cast<const BranchInst&>(inst));
        }
        else if(inst.op==OpType::Jump){
            visitJump(static_cast<const JumpInst&>(inst));
        }
        else if(inst.op==OpType::Call){
            visitCall(static_cast<const CallInst&>(inst));
        }
        else if(inst.op==OpType::GetElemPtr){
            visitGetElemPtr(static_cast<const GetElemPtrInst&>(inst));
        }

         else {
            visitBinary(static_cast<const Binary&>(inst));
        }
    }
    void visitAlloc(const AllocInst& inst) {
        //不生成具体指令
    }
    void visitStore(const StoreInst& inst) {
    //store 10,@x.  把 10加载到临时寄存器，然后存到栈上
    // 准备好要存的值
    std::string valReg = getValRegFromStack(inst.value, "t0");

    if (inst.address->isGlobal()) {
        // 存入全局变量：la -> sw
        ss << "  la t1, " << inst.address->name.substr(1) << "\n";
        ss << "  sw " << valReg << ", 0(t1)\n";
    } 
    else {
        const Instruction* addrInst = dynamic_cast<const Instruction*>(inst.address);
        // 情况 A：直接存入局部标量或数组 (AllocInst)
        // 这里的地址就是 sp + offset
        if (addrInst && addrInst->op == OpType::Alloc) {
            int offset = getStackOffset(inst.address->name);
            ss << "  sw " << valReg << ", " << offset << "(sp)\n";
        } 
        // 情况 B：存入计算出来的地址 (GetElemPtrInst 的结果)
        // 这里的地址值已经存在栈上了，需要先 lw 出来
        else {
            int addrOffset = getStackOffset(inst.address->name);
            ss << "  lw t1, " << addrOffset << "(sp)\n"; // 拿到算好的地址
            ss << "  sw " << valReg << ", 0(t1)\n";      // 往那个地址存货
        }
    }
    }
    
 void visitLoad(const LoadInst& inst) {
    int offsetDest = getStackOffset(inst.name);
    // 1. 处理全局变量 (@x)
    if (inst.address->isGlobal()) {
        ss << "  la t0, " << inst.address->name.substr(1) << "\n"; // 拿物理地址
        ss << "  lw t0, 0(t0)\n";                                 // 从该地址取货
    } 
    else {
        const Instruction* addrInst = dynamic_cast<const Instruction*>(inst.address);
        // 情况 A：直接加载局部变量 (AllocInst)
        // 地址就是固定的 sp + offset
        if (addrInst && addrInst->op == OpType::Alloc) {
            int offsetSrc = getStackOffset(inst.address->name);
            ss << "  lw t0, " << offsetSrc << "(sp)\n";           // 一步到位取货
        } 
        // 情况 B：从指针/GEP 结果加载 (GetElemPtrInst)
        // 栈里存的是地址，需要两次lw
        else {
            int addrOffset = getStackOffset(inst.address->name);
            ss << "  lw t1, " << addrOffset << "(sp)\n";         
            ss << "  lw t0, 0(t1)\n";                             
        }
    }
    // 最后把取到的货 (t0) 存到目标变量在栈上的坑里
    ss << "  sw t0, " << offsetDest << "(sp)\n";
}
    void visitBranch(const BranchInst& inst) {
        //br %cond, %then, %else
        //bnez %cond, then
        //j else
        std::string condReg = getValRegFromStack(inst.condition,"t0");
        ss << "  bnez " << condReg << ", "<< getAsmBlockLabel(inst.thenBlock->name) << "\n";
        ss << "  j " << getAsmBlockLabel(inst.elseBlock->name) << "\n";
    }
    void visitJump(const JumpInst& inst) {
        //jump %target
        ss << "  j " << getAsmBlockLabel(inst.targetBlock->name) << "\n";
    }
    void visitBinary(const Binary& inst) {
        std::string rs1= getValRegFromStack(inst.lhs,"t0");
        std::string rs2= getValRegFromStack(inst.rhs,"t1");

        if (inst.op == OpType::Sub) {
            ss << "  sub t0, " <<rs1 << ", " << rs2 << "\n";
        } 
        else if (inst.op == OpType::Add) {
            ss << "  add t0, " << rs1 << ", " << rs2 << "\n";
        }
        else if (inst.op == OpType::Eq) {
            ss << "  xor t0, " << rs1 << ", " << rs2 << "\n";
            ss << "  seqz t0, " << "t0" << "\n";
        }
        else if (inst.op == OpType::Mul) {
            ss << "  mul t0, " << rs1 << ", " << rs2 << "\n";
        }
        else if (inst.op == OpType::Div) {
            ss << "  div t0, " << rs1 << ", " << rs2 << "\n";
        }
        else if (inst.op == OpType::Mod) {
            ss << "  rem t0, " << rs1 << ", " << rs2 << "\n";
        }
        else if (inst.op == OpType::Lt) {
            ss << "  slt t0, " << rs1 << ", " << rs2 << "\n";
        }
        else if (inst.op == OpType::Gt) {
            ss << "  sgt t0, " << rs1 << ", " << rs2 << "\n";
        }
        else if (inst.op == OpType::Le) {
            ss << "  sgt t0, " << rs1 << ", " << rs2 << "\n";
            ss << "  seqz t0, " << "t0" << "\n";
        }
        else if (inst.op == OpType::Ge) {
            ss << "  slt t0, " << rs1 << ", " << rs2 << "\n";
            ss << "  seqz t0, " << "t0" << "\n";
        }
        else if (inst.op == OpType::Ne) {
            ss << "  xor t0, " << rs1 << ", " << rs2 << "\n";
            ss << "  snez t0, " << "t0" << "\n";
        }
        else if (inst.op == OpType::AND) {
            ss << "  and t0, " << rs1 << ", " << rs2 << "\n";
        }
        else if (inst.op == OpType::OR) {
            ss << "  or t0, " << rs1 << ", " << rs2 << "\n";
        }
        int offset = getStackOffset(inst.name);
        ss << "  sw t0, " << offset << "(sp)\n";

    }
    void visitReturn(const ReturnInst& inst) {
       if (inst.retValue) { 
        std::string valReg = getValRegFromStack(inst.retValue, "a0");
        if (valReg != "a0") {
            ss << "  mv a0, " << valReg << "\n";
        }
        }
        if (currentLayout.R > 0) {
        ss << "  lw ra, " << currentLayout.raOffset << "(sp)\n";
    }
        int total=currentLayout.total;
        if (total > 0) {
            if (total <= 2048) {
            ss << "  addi sp, sp, " << total << "\n";
            }
            else {
            ss << "  li t0, " << total << "\n";
            ss << "  add sp, sp, t0\n";
            }
    }
        ss << "  ret\n";
    }
    
    void visitCall(const CallInst& inst) {
        int argCount = inst.args.size();
        for (int i = 0; i < std::min(argCount, 8); ++i) {

        std::string targetReg = "a" + std::to_string(i);
        std::string srcReg = getValRegFromStack(inst.args[i], targetReg);

         if (srcReg != targetReg) {
            ss << "  mv " << targetReg << ", " << srcReg << "\n";
        }
        }

        if (argCount > 8) {
        for (int i = 8; i < argCount; ++i) {
            std::string srcReg = getValRegFromStack(inst.args[i], "t0");
            int offset = (i - 8) * 4; 
            ss << "  sw " << srcReg << ", " << offset << "(sp)\n";
        }
    }

        ss << "  call " << inst.funcName.substr(1) << "\n";

        if (inst.type != Type::Void) {
            int offset = getStackOffset(inst.name);
            ss << "  sw a0, " << offset << "(sp)\n";
        }
    }

    void visitGetElemPtr(const GetElemPtrInst& inst) {
    // 1. 获取基地址（放到 t0）
    if (inst.ptr->isGlobal()) {
        ss << "  la t0, " << inst.ptr->name.substr(1) << "\n";
    } else {
        const Instruction* ptrInst = dynamic_cast<const Instruction*>(inst.ptr);

        if (ptrInst && ptrInst->op == OpType::Alloc) {
            int offset = getStackOffset(inst.ptr->name);
            ss << "  addi t0, sp, " << offset << "\n";
        } else {
            // 如果不是 alloc（比如是上一个 GEP 算出的地址），
            // 那么这个“值”本身就存在栈上，我们需要用 lw 把它读出来
            int offset = getStackOffset(inst.ptr->name);
            ss << "  lw t0, " << offset << "(sp)\n";
        }
    }
    // 2. 获取下标并计算偏移（逻辑保持不变）
    std::string idxReg = getValRegFromStack(inst.index, "t1");
    ss << "  slli t1, " << idxReg << ", 2\n"; // t1 = idx * 4
    ss << "  add t0, t0, t1\n";              // t0 = 基址 + 偏移
    // 3. 将算出的地址存回栈
    int destOffset = getStackOffset(inst.name);
    ss << "  sw t0, " << destOffset << "(sp)\n";
}
};
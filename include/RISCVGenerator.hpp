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
    StackLayout() : A(0), S(0), R(0), total(0), raOffset(0) {}
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
            if (inst->op == OpType::Alloc||inst->type == Type::Int32) {
                stackMap[inst->name]=offset;
                offset+=4; 
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
public:
    std::string generate(const Program& prog) {
        ss.str(""); 

        ss << "  .text\n";
        ss << "  .globl main\n";

        for (const auto& func : prog.funcs) {
            visit(*func);
        }
        
        return ss.str();
    }

   std::string getValRegFromStack(Value* val, const std::string& tempReg) {
    std::string name=val->name;
    if(name=="0"){
        return "x0";
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
        currentLayout = computeLayout(func, stackMap);
        int total=currentLayout.total;
        
        ss<<func.name.substr(1) << ":\n";
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
        ss<<block.name.substr(1)<< ":\n";
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
         else {
            visitBinary(static_cast<const Binary&>(inst));
        }
    }
    void visitAlloc(const AllocInst& inst) {
        //不生成具体指令
    }
    void visitStore(const StoreInst& inst) {
        //store 10,@x.  把 10加载到临时寄存器，然后存到栈上
        std::string valReg = getValRegFromStack(inst.value,"t0");
        int offset = getStackOffset(inst.address->name);
        ss << "  sw " << valReg << ", " << offset << "(sp)\n";
    }
    void visitLoad(const LoadInst& inst) {
        //%0 = load @x
        //从栈上加载到一个临时寄存器
        int offset1 = getStackOffset(inst.address->name);
        int offset2 = getStackOffset(inst.name);
        ss << "  lw t0, " << offset1 << "(sp)\n";
        ss << "  sw t0, " << offset2 << "(sp)\n"; 
    }
    void visitBranch(const BranchInst& inst) {
        //br %cond, %then, %else
        //bnez %cond, then
        //j else
        std::string condReg = getValRegFromStack(inst.condition,"t0");
        ss << "  bnez " << condReg << ", "<< inst.thenBlock->name.substr(1) << "\n";
        ss << "  j " << inst.elseBlock->name.substr(1) << "\n";
    }
    void visitJump(const JumpInst& inst) {
        //jump %target
        ss << "  j " << inst.targetBlock->name.substr(1) << "\n";
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
};
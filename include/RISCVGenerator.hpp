#pragma once
#include "ir.hpp"
#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>

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
private:

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
    void allocateStack(const Function& func){
        int offset=0;
        for(const auto& block : func.blocks) {
            for (const auto& inst : block->insts) {
                if (inst->op == OpType::Alloc) {
                    stackMap[inst->name]=offset;
                    offset+=4; 
                }
                else if (inst->type == Type::Int32) {
                    stackMap[inst->name]=offset;
                    offset+=4; 
                }
            }
        }
        stackSize = offset;
        if(stackSize%16!=0){
            stackSize= ((stackSize/16)+1)*16;
        }
    }


    void visit(const Function& func) {
        stackMap.clear();
        stackSize=0;
        allocateStack(func);
        ss<<func.name.substr(1) << ":\n";
        if (stackSize > 0) {
        if (stackSize <= 2048) {
            ss << "  addi sp, sp, -" << stackSize << "\n";
        } else {
            ss << "  li t0, -" << stackSize << "\n";
            ss << "  add sp, sp, t0\n";
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
        std::string valReg = getValRegFromStack(inst.retValue,"a0");
        if (stackSize > 0) {
        if (stackSize <= 2048) {
            ss << "  addi sp, sp, " << stackSize << "\n";
        } else {
            ss << "  li t0, " << stackSize << "\n";
            ss << "  add sp, sp, t0\n";
        }
    }
        ss << "  ret\n";
    }
};
#pragma once
#include "ir.hpp"
#include <map>
#include <string>
#include <sstream>
#include <iostream>

class RISCVGenerator {
private:
    // 1. 这是我们的笔记本，生成的代码先写在这里
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

    std::map<std::string, std::string> regMap;
    int tCounter=0;

    std::string allocReg(const std::string& name) {
    std::string reg = "t" + std::to_string(tCounter);
    tCounter++;   
    regMap[name] = reg; 
    return reg;
    }
    
    std::string getReg(const std::string& name) {
        if (regMap.find(name) != regMap.end()) {
            return regMap[name];
        }
        return "error_reg"; 
    }


    std::string getValReg(Value* val,std::string tmpReg="t6") {
       std::string name=val->name;
       if (name[0] == '%') {
            return getReg(name);
        }
        else if(name=="0"){
            return "x0";
        }
        //如果是一个非零的立即数，输出 li 指令
        ss<< "  li " << tmpReg << ", " << name << "\n";
        return tmpReg;
    }

    void visit(const Function& func) {
        ss<<func.name.substr(1) << ":\n";
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
        } else {
            visitBinary(static_cast<const Binary&>(inst));
        }
    }
    void visitBinary(const Binary& inst) {
        std::string rd = allocReg(inst.name);

        std::string rs1 = getValReg(inst.lhs,"t5");
        std::string rs2 = getValReg(inst.rhs,"t6");

        if (inst.op == OpType::Sub) {
            ss << "  sub " << rd << ", " << rs1 << ", " << rs2 << "\n";
        } 
        else if (inst.op == OpType::Add) {
            ss << "  add " << rd << ", " << rs1 << ", " << rs2 << "\n";
        }
        else if (inst.op == OpType::Eq) {
            ss << "  xor "<< rd << ", " << rs1 << ", " << rs2 << "\n";
            ss << "  seqz " << rd << ", " << rd << "\n";
        }
        else if (inst.op == OpType::Mul) {
            ss << "  mul " << rd << ", " << rs1 << ", " << rs2 << "\n";
        }
        else if (inst.op == OpType::Div) {
            ss << "  div " << rd << ", " << rs1 << ", " << rs2 << "\n";
        }
        else if (inst.op == OpType::Mod) {
            ss << "  rem " << rd << ", " << rs1 << ", " << rs2 << "\n";
        }
    }
    void visitReturn(const ReturnInst& inst) {
        std::string valReg = getValReg(inst.retValue);
        ss << "  mv a0, " << valReg << "\n";
        ss << "  ret\n";
    }
};
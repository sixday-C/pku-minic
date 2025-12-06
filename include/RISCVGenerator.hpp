#pragma once
#include "ir.hpp"
#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>

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
    const std::vector<std::string> regs = {
        "t0", "t1", "t2", "t3", "t4", "t5", "t6",
        "a1", "a2", "a3", "a4", "a5", "a6", "a7",
        "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11"
    };
    std::string allocReg(const std::string& name) {
    if (tCounter >= regs.size()) {
            tCounter = 0; 
        }
        std::string reg = regs[tCounter];
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


    std::string getValReg(Value* val,std::string tmpReg="s0") {
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

        std::string rs1 = getValReg(inst.lhs,"s0");
        std::string rs2 = getValReg(inst.rhs,"s1");

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
        else if (inst.op == OpType::Lt) {
            ss << "  slt " << rd << ", " << rs1 << ", " << rs2 << "\n";
        }
        else if (inst.op == OpType::Gt) {
            ss << "  sgt " << rd << ", " << rs1 << ", " << rs2 << "\n";
        }
        else if (inst.op == OpType::Le) {
            ss << "  sgt " << rd << ", " << rs1 << ", " << rs2 << "\n";
            ss << "  seqz " << rd << ", " << rd << "\n";
        }
        else if (inst.op == OpType::Ge) {
            ss << "  slt " << rd << ", " << rs1 << ", " << rs2 << "\n";
            ss << "  seqz " << rd << ", " << rd << "\n";
        }
        else if (inst.op == OpType::Ne) {
            ss << "  xor "<< rd << ", " << rs1 << ", " << rs2 << "\n";
            ss << "  snez " << rd << ", " << rd << "\n";
        }
        else if (inst.op == OpType::AND) {
            ss << "  and " << rd << ", " << rs1 << ", " << rs2 << "\n";
        }
        else if (inst.op == OpType::OR) {
            ss << "  or " << rd << ", " << rs1 << ", " << rs2 << "\n";
        }

    }
    void visitReturn(const ReturnInst& inst) {
        std::string valReg = getValReg(inst.retValue);
        ss << "  mv a0, " << valReg << "\n";
        ss << "  ret\n";
    }
};
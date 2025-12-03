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


    std::string getValReg(Value* val) {
       std::string name=val->name;
       if (name[0] == '%') {
            return getReg(name);
        }
        else if(name=="0"){
            return "x0";
        }
        //如果是一个非零的立即数，输出 li 指令
        ss<< "  li t6, " << name << "\n";
        return "t6";
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

        std::string rs1 = getValReg(inst.lhs);
        std::string rs2 = getValReg(inst.rhs);

        if (inst.op == OpType::Sub) {
            ss << "  sub " << rd << ", " << rs1 << ", " << rs2 << "\n";
        } 
        else if (inst.op == OpType::Add) {
            ss << "  add " << rd << ", " << rs1 << ", " << rs2 << "\n";
        }
        else if (inst.op == OpType::Eq) {
            // 1. xor rd, rs1, rs2  (如果不相等，rd非0；如果相等，rd为0)
            ss << "  xor "<< rd << ", " << rs1 << ", " << rs2 << "\n";
            // 2. seqz rd, rd       (如果rd为0，则置1；否则置0)
            ss << "  seqz " << rd << ", " << rd << "\n";
        }
    }
    void visitReturn(const ReturnInst& inst) {
        std::string valReg = getValReg(inst.retValue);
        
        // mv a0, ...
        ss << "  mv a0, " << valReg << "\n";
        ss << "  ret\n";
    }
};
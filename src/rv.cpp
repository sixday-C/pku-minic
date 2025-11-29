#include "../include/rv.hpp"
#include <iostream>

// ============================================
// ---- 加载和移动类指令的输出实现 ----
// ============================================

// 加载立即数指令：li rd, imm
void RVLi::emit(std::ostream& os) const {
    os << "  li " << rd.name << ", " << imm.v;
}

// 移动指令：mv rd, rs 
void RVMv::emit(std::ostream& os) const {
    os << "  mv " << rd.name << ", " << rs.name;
}

// ============================================
// ---- 控制转移类指令的输出实现 ----
// ============================================

// 返回伪指令：ret → jr ra
void RVRet::emit(std::ostream& os) const {
    os << "  ret";
}

// ============================================
// ---- 运算类指令的输出实现 ----
// ============================================

// 立即数加法指令：addi rd, rs, imm  
void RVAddi::emit(std::ostream& os) const {
    os << "  addi " << rd.name << ", " << rs.name << ", " << imm.v;
}

// 寄存器加法指令：add rd, rs1, rs2
void RVAdd::emit(std::ostream& os) const {
    os << "  add " << rd.name << ", " << rs1.name << ", " << rs2.name;
}

// 寄存器减法指令：sub rd, rs1, rs2
void RVSub::emit(std::ostream& os) const {
    os << "  sub " << rd.name << ", " << rs1.name << ", " << rs2.name;
}

// 取负伪指令：neg rd, rs → sub rd, x0, rs
void RVNeg::emit(std::ostream& os) const {
    os << "  sub " << rd.name << ", x0, " << rs.name;
}

// 异或运算指令：xor rd, rs1, rs2
void RVXor::emit(std::ostream& os) const {
    os << "  xor " << rd.name << ", " << rs1.name << ", " << rs2.name;
}

// 等于零伪指令：seqz rd, rs
void RVSeqz::emit(std::ostream& os) const {
    os << "  seqz " << rd.name << ", " << rs.name;
}

// ============================================
// ---- 顶层输出函数实现 ----
// ============================================

// 输出基本块：先输出标签，然后输出所有指令
void emitBlock(const RVBlock& bb, std::ostream& os) {
    // 输出基本块标签
    os << bb.label << ":\n";
    
    // 依次输出所有指令
    for (const auto& inst : bb.insts) {
        inst->emit(os);
        os << "\n";
    }
}

// 输出函数：包含函数声明、标签和所有基本块
void emitFunc(const RVFunc& fn, std::ostream& os, bool minimal_prologue) {
    // 输出全局函数声明
    os << "  .globl " << fn.name << "\n";
    
    // 输出函数标签
    os << fn.name << ":\n";
    
    // 输出所有基本块
    for (const auto& block : fn.blocks) {
        // 对于第一个基本块，不输出额外标签
        if (block == fn.blocks.front()) {
            // 只输出指令
            for (const auto& inst : block->insts) {
                inst->emit(os);
                os << "\n";
            }
        } else {
            // 其他基本块正常输出
            emitBlock(*block, os);
        }
    }
}

// 输出整个程序：包含 .text 段声明和所有函数
void emitProgram(const RVProg& pg, std::ostream& os, bool minimal_prologue) {
    // 输出汇编文件头部
    os << "  .text\n";
    
    // 依次输出所有函数
    for (const auto& func : pg.funcs) {
        emitFunc(*func, os, minimal_prologue);
        os << "\n";  // 函数之间添加空行，提高可读性
    }
}

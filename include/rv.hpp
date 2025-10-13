// 最小化、可维护的 RISC-V IR 结构 + 输出 API
// =============================
#pragma once
#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

// ---- 操作数 ----
struct RVReg {               // 寄存器（可以是虚拟寄存器如 "%v0" 或物理寄存器如 "t0"）
  std::string name;          // 例如："a0", "t0", "%v3"
};

struct RVImm {               // 立即数
  int32_t v{0};
};

// ---- 指令（抽象基类）----
struct RVInst {
  virtual ~RVInst() = default;
  virtual void emit(std::ostream& os) const = 0;  // 输出汇编代码（可能展开伪指令）
};

// 保持一个小而实用的指令集（允许伪指令；在 emit 中展开）
struct RVLi   : RVInst { RVReg rd; RVImm imm; void emit(std::ostream& os) const override; };  // 加载立即数
struct RVAddi : RVInst { RVReg rd, rs; RVImm imm; void emit(std::ostream& os) const override; }; // 立即数加法
struct RVAdd  : RVInst { RVReg rd, rs1, rs2;   void emit(std::ostream& os) const override; };   // 寄存器加法
struct RVSub  : RVInst { RVReg rd, rs1, rs2;   void emit(std::ostream& os) const override; };   // 寄存器减法
struct RVXor  : RVInst { RVReg rd, rs1, rs2;   void emit(std::ostream& os) const override; };   // 异或运算
struct RVMv   : RVInst { RVReg rd, rs;         void emit(std::ostream& os) const override; };   // 伪指令：移动 → addi rd, rs, 0
struct RVNeg  : RVInst { RVReg rd, rs;         void emit(std::ostream& os) const override; };   // 伪指令：取负 → sub rd, x0, rs
struct RVSeqz : RVInst { RVReg rd, rs;         void emit(std::ostream& os) const override; };   // 伪指令：等于零 → sltiu rd, rs, 1
struct RVRet  : RVInst {                        void emit(std::ostream& os) const override; };   // 伪指令：返回 → jr ra

// ---- 控制流图容器 ----
struct RVBlock {
  std::string label;                               // 例如："entry"
  std::vector<std::unique_ptr<RVInst>> insts;      // 指令的线性列表
};

struct RVFunc {
  std::string name;                                // 例如："main"
  std::vector<std::unique_ptr<RVBlock>> blocks;    // 基本块顺序对输出很重要
};

struct RVProg {
  std::vector<std::unique_ptr<RVFunc>> funcs;      // 函数列表
};

// ---- 小型构建器辅助函数 ----

// 模板函数：向指令列表中添加新指令
template <class T, class... Args>
inline T* rv_emit(std::vector<std::unique_ptr<RVInst>>& into, Args&&... args) {
  auto p = std::make_unique<T>(T{std::forward<Args>(args)...});
  T* raw = p.get();
  into.emplace_back(std::move(p));
  return raw;
}

// 向函数中添加新的基本块
inline RVBlock* rv_new_block(RVFunc& f, std::string label) {
  auto blk = std::make_unique<RVBlock>(); blk->label = std::move(label);
  auto* raw = blk.get(); f.blocks.emplace_back(std::move(blk)); return raw;
}

// 向程序中添加新函数
inline RVFunc* rv_new_func(RVProg& p, std::string name) {
  auto fn = std::make_unique<RVFunc>(); fn->name = std::move(name);
  auto* raw = fn.get(); p.funcs.emplace_back(std::move(fn)); return raw;
}

// ---- 顶层输出函数 ----
void emitBlock  (const RVBlock&  bb, std::ostream& os);                           // 输出基本块
void emitFunc   (const RVFunc&   fn, std::ostream& os, bool minimal_prologue = true); // 输出函数
void emitProgram(const RVProg&   pg, std::ostream& os, bool minimal_prologue = true); // 输出整个程序

// =============================
#include "../include/rv_gen.hpp"
#include <cassert>

// 顶层：遍历所有函数，输出 .text 段
void RvGen::generate(const Program& p) {
  os_ << "  .text\n";
  for (const auto& f : p.funcs) {
    gen_function(*f);
  }
}

// 函数：输出全局符号和入口标签，然后遍历基本块
void RvGen::gen_function(const Function& f) {
  os_ << "  .globl " << f.name << "\n";
  os_ << f.name << ":\n";
  for (const auto& bb : f.blocks) {
    gen_block(*bb);
  }
}

// 基本块：标签
void RvGen::gen_block(const BasicBlock& bb) {
  os_ << bb.name << ":\n";
  for (const auto& inst : bb.insts) {
    // 现在仅支持 Return 指令
    if (auto* ret = dynamic_cast<Return*>(inst.get())) {
      if (ret->value) eval_to_a0(*ret->value);  // 把返回值放到 a0
      os_ << "  ret\n";
    }
    // 以后新增其它指令就在这里扩分支
  }
}

// 把一个 Value 结果放入 a0：当前只支持整数字面量
void RvGen::eval_to_a0(const Value& v) {
  if (auto* ci = dynamic_cast<const ConstInt*>(&v)) {
    os_ << "  li a0, " << ci->v << "\n";
    return;
  }
  assert(false && "RvGen::eval_to_a0: unsupported Value kind");
}
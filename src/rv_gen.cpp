#include "../include/rv_gen.hpp"
#include <map>
#include <cassert>

// 生成整个程序的 RISC-V 汇编
void RvGen::generate(const Program& p) {
  os_ << "  .text\n";
  
  for (const auto& f : p.funcs) {
    gen_function(*f);
  }
}

// 生成函数的汇编
void RvGen::gen_function(const Function& f) {
  // 输出函数声明和标签
  os_ << "  .globl " << f.name << "\n";
  os_ << f.name << ":\n";
  
  // 重置寄存器分配状态
  reg_counter_ = 0;
  temp_to_reg_.clear();
  
  for (const auto& bb : f.blocks) {
    gen_block(*bb);
  }
}

// 生成基本块的汇编
void RvGen::gen_block(const BasicBlock& bb) {
  // entry 基本块不需要额外标签（函数名已经是标签）
  if (bb.name != "entry") {
    os_ << bb.name << ":\n";
  }
  
  for (const auto& inst : bb.insts) {
    // 处理二元运算指令
    if (auto* binary = dynamic_cast<const Binary*>(inst.get())) {
      gen_binary(*binary);
    }
    // 处理返回指令
    else if (auto* ret = dynamic_cast<const Return*>(inst.get())) {
      gen_return(*ret);
    }
  }
}


// 处理二元运算指令

// 处理二元运算指令
void RvGen::gen_binary(const Binary& binary) {
  std::string rd = get_reg_name(binary.name);  // 为结果分配寄存器
  
  if (binary.op == Binary::EQ) {
    // 优化：检查右操作数是否为 0
    if (auto* rhs_int = dynamic_cast<const Integer*>(binary.rhs.get())) {
      if (rhs_int->value == 0) {
        // eq x, 0 → 直接生成到结果寄存器
        if (auto* lhs_int = dynamic_cast<const Integer*>(binary.lhs.get())) {
          // eq 常量, 0
          os_ << "  li   " << rd << ", " << lhs_int->value << "\n";
          os_ << "  seqz " << rd << ", " << rd << "\n";
        } else if (auto* lhs_var = dynamic_cast<const Variable*>(binary.lhs.get())) {
          // eq 变量, 0
          std::string r1 = get_reg_name(lhs_var->name);
          os_ << "  seqz " << rd << ", " << r1 << "\n";
        }
        return;
      }
    }
    
    // 通用情况：eq x, y → sub + seqz
    std::string r1 = ensure_in_reg(*binary.lhs);
    std::string r2 = ensure_in_reg(*binary.rhs);
    os_ << "  sub  " << rd << ", " << r1 << ", " << r2 << "\n";
    os_ << "  seqz " << rd << ", " << rd << "\n";
  }
  else if (binary.op == Binary::SUB) {
    // sub 优化：sub 0, x → neg
    if (auto* lhs_int = dynamic_cast<const Integer*>(binary.lhs.get())) {
      if (lhs_int->value == 0) {
        // sub 0, x → neg
        if (auto* rhs_int = dynamic_cast<const Integer*>(binary.rhs.get())) {
          // sub 0, 常量
          os_ << "  li   " << rd << ", " << (-rhs_int->value) << "\n";
        } else if (auto* rhs_var = dynamic_cast<const Variable*>(binary.rhs.get())) {
          // sub 0, 变量
          std::string r2 = get_reg_name(rhs_var->name);
          os_ << "  sub  " << rd << ", x0, " << r2 << "\n";
        }
        return;
      }
    }
    
    // 通用情况：sub x, y
    std::string r1 = ensure_in_reg(*binary.lhs);
    std::string r2 = ensure_in_reg(*binary.rhs);
    os_ << "  sub  " << rd << ", " << r1 << ", " << r2 << "\n";
  }
}

// 处理返回指令
void RvGen::gen_return(const Return& ret) {
  // 将返回值移动到 a0 寄存器
  if (ret.value) {
    if (auto* int_val = dynamic_cast<const Integer*>(ret.value.get())) {
      // 返回常量
      os_ << "  li   a0, " << int_val->value << "\n";
    } else if (auto* var = dynamic_cast<const Variable*>(ret.value.get())) {
      // 返回变量
      std::string src_reg = get_reg_name(var->name);
      os_ << "  mv   a0, " << src_reg << "\n";
    }
  }
  else if (!ret.temp_name.empty()) {
    std::string temp_reg = get_reg_name(ret.temp_name);
    os_ << "  mv   a0, " << temp_reg << "\n";
  }
  
  os_ << "  ret\n";
}

// 简化的 ensure_in_reg：只用于通用情况
std::string RvGen::ensure_in_reg(const Value& v) {
  if (auto* i = dynamic_cast<const Integer*>(&v)) {
    // 0 直接使用 x0
    if (i->value == 0) return "x0";
    
    // 其他常量：报错，不应该走到这里
    // 常量应该在 gen_binary 中直接处理
    return "x0";
  }
  
  if (auto* var = dynamic_cast<const Variable*>(&v)) {
    return get_reg_name(var->name);
  }
  
  return "t0";
}

// 获取临时变量对应的寄存器名
std::string RvGen::get_reg_name(const std::string& temp_var) {
  if (temp_to_reg_.find(temp_var) != temp_to_reg_.end()) {
    return temp_to_reg_[temp_var];
  }
  
  std::string reg_name = "t" + std::to_string(reg_counter_++);
  temp_to_reg_[temp_var] = reg_name;
  return reg_name;
}

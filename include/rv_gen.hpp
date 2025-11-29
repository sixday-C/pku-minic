#pragma once
#include <memory>
#include <ostream>
#include "ir.hpp"   // Koopa IR 定义
#include <map>

// RISC-V 代码生成器：将 Koopa IR 直接输出为 RISC-V 汇编文本
class RvGen {
public:
  explicit RvGen(std::ostream& os) : os_(os), reg_counter_(0) {}
  
  // 主入口：生成整个程序的 RISC-V 汇编
  void generate(const Program& p);

private:
  std::ostream& os_;  // 输出流
  
  // 生成函数的汇编
  void gen_function(const Function& f);
  
  // 生成基本块的汇编
  void gen_block(const BasicBlock& bb);
  
  // 处理二元运算指令，输出相应的 RISC-V 指令
  void gen_binary(const Binary& binary);
  
  // 处理返回指令，输出相应的 RISC-V 指令
  void gen_return(const Return& ret);
  
  // 辅助函数：将 Value 加载到寄存器（可能输出 li 指令）
  std::string ensure_in_reg(const Value& v);
  
  // 辅助函数：获取临时变量对应的寄存器名
  std::string get_reg_name(const std::string& temp_var);
  
  // 寄存器分配计数器和映射表
  int reg_counter_;
  std::map<std::string, std::string> temp_to_reg_;
};

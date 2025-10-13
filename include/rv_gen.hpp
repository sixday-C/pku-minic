#pragma once
#include <memory>
#include "ir.hpp"   // Koopa IR 定义
#include "rv.hpp"   // RISC-V 内存结构定义
#include <map>

// RISC-V 代码生成器：将 Koopa IR 转换为 RISC-V 内存结构
class RvGen {
public:
  RvGen() : reg_counter_(0) {}
  
  // 主入口：生成整个程序的 RISC-V 内存结构
  std::unique_ptr<RVProg> generate(const Program& p);

private:
  // 生成函数的 RISC-V 结构
  std::unique_ptr<RVFunc> gen_function(const Function& f);
  
  // 生成基本块的 RISC-V 结构
  std::unique_ptr<RVBlock> gen_block(const BasicBlock& bb);
  
  // 处理二元运算指令，生成相应的 RISC-V 指令
  void gen_binary(const Binary& binary, std::vector<std::unique_ptr<RVInst>>& insts);
  
  // 处理返回指令，生成相应的 RISC-V 指令
  void gen_return(const Return& ret, std::vector<std::unique_ptr<RVInst>>& insts);
  
  // 辅助函数：获取临时变量对应的寄存器名
  std::string get_reg_name(const std::string& temp_var);
  
  // 寄存器分配计数器和映射表
  int reg_counter_;
  std::map<std::string, std::string> temp_to_reg_;
};

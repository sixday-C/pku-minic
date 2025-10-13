#pragma once
#include <memory>
#include "ast.hpp"  // AST 节点定义
#include "ir.hpp"   // IR 节点定义

// IR 生成器：将 AST 转换为 IR
// 采用直接递归方式遍历 AST，为每种节点类型提供对应的处理函数
// 支持一元表达式生成（+, -, !）和常量表达式
class IRGenerator {
public:
  // 主入口：将 AST 转换为 IR Program
  std::unique_ptr<Program> Generate(const CompUnitAST& ast);

private:
  // 当前写入上下文
  Function*   current_func  = nullptr;
  BasicBlock* current_block = nullptr;
  
  // 临时变量计数器
  int temp_counter = 0;
  
  // 生成的 Program
  std::unique_ptr<Program> program;

  // 简化的表达式生成，类似原来的 AST 方式
  void generate_expr(const BaseAST& expr);  // 生成表达式 IR，结果保存在 last_value
  
  // 辅助函数
  std::string new_temp();  // 生成新的临时变量名 %0, %1, %2...
  std::string last_value;  // 保存最后生成的值（临时变量名或常量值）
  bool is_constant = false; // 标记 last_value 是否为常量
};
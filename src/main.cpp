#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <utility>
#include "../include/ast.hpp"
#include "../include/ir_gen.hpp"
#include "../include/ir.hpp"
#include "../include/rv_gen.hpp"
using namespace std;

// 声明 lexer 的输入, 以及 parser 函数
// 为什么不引用 sysy.tab.hpp 呢? 因为首先里面没有 yyin 的定义
// 其次, 因为这个文件不是我们自己写的, 而是被 Bison 生成出来的
// 你的代码编辑器/IDE 很可能找不到这个文件, 然后会给你报错 (虽然编译不会出错)
// 看起来会很烦人, 于是干脆采用这种看起来 dirty 但实际很有效的手段
extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);

int main(int argc, const char *argv[]) {
  if (argc < 5) {
    std::cerr << "Usage: " << argv[0] << " -koopa <input_file> -o <output_file>" << std::endl;
    std::cerr << "       "<< argv[0] << " -riscv <input_file> -o <output_file>" << std::endl;
    return 1;
  }

  auto mode = std::string(argv[1]);
  auto input = argv[2];
  auto output = argv[4];

  yyin = fopen(input, "r");
  assert(yyin);

  unique_ptr<BaseAST> ast;
  auto ret = yyparse(ast);
  assert(!ret);

  // ast->Dump();
  // cout << endl;

  IRGenerator generator;
  CompUnitAST* comp_unit_ast = static_cast<CompUnitAST*>(ast.get()); 
  auto koopa_program = generator.Generate(*comp_unit_ast);

  std::ofstream output_file(output);
  if (!output_file.is_open()) {
    std::cerr << "Error: Could not open output file " << output << std::endl;
    return 1;
  }

  if (mode == "-koopa") {
    koopa_program->dump(output_file);
    std::cout << "Successfully generated Koopa IR to " << output << std::endl;
  } 
  else if (mode == "-riscv") {
    RvGen riscv_generator(output_file);
    riscv_generator.generate(*koopa_program);
    std::cout << "Successfully generated RISCV assembly to " << output << std::endl;
  }
  else {
    std::cerr << "Error: Unkown mode " << mode << std::endl;
    return 1;
  }
  
  return 0;
}
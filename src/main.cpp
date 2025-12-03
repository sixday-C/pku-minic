#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <utility>
#include "../include/ast.hpp"
#include "../include/IRGenerator.hpp"
#include "../include/ir.hpp"
#include "../include/RISCVGenerator.hpp"
//#include "../include/rv_gen.hpp"
using namespace std;

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

  IRGenerator generator;
  CompUnitAST* comp_unit_ast = static_cast<CompUnitAST*>(ast.get()); 
  generator.visit(comp_unit_ast);
  auto koopa_program = generator.getProgram();

  std::ofstream output_file(output);
  if (!output_file.is_open()) {
    std::cerr << "Error: Could not open output file " << output << std::endl;
    return 1;
  }

  if (mode == "-koopa") {
    koopa_program->toString(output_file);
    std::cout << "Successfully generated Koopa IR to " << output << std::endl;
  } 
  else if (mode == "-riscv") {
    RISCVGenerator riscv_generator;
    std::string riscv_code = riscv_generator.generate(*koopa_program);
    output_file << riscv_code;
    std::cout << "Successfully generated RISCV assembly to " << output << std::endl;
  }
  else {
    std::cerr << "Error: Unknown mode " << mode << std::endl;
    return 1;
  }
  
  return 0;
}
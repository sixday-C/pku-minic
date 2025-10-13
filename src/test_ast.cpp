// //只用来打印AST
// //检测语法分析器是否正确
// #include "../include/ast.hpp"


// void print_ast(const BaseAST& ast) {
//   ast.Dump();
// }
// int main() {
//   // 创建一个简单的 AST 节点
//   auto number_node = std::make_unique<NumberAST>(42);
//   auto stmt_node = std::make_unique<StmtAST>();
//   stmt_node->Exp = std::move(number_node);
//   auto func_type_node = std::make_unique<FuncTypeAST>("int");
//   auto block_node = std::make_unique<BlockAST>();
//   block_node->stmt = std::move(stmt_node);
//   auto func_def_node = std::make_unique<FuncDefAST>();
//   func_def_node->func_type = std::move(func_type_node);
//   func_def_node->ident = "main";
//   func_def_node->block = std::move(block_node);
//   auto comp_unit_node = std::make_unique<CompUnitAST>();
//   comp_unit_node->func_def = std::move(func_def_node);

//   // 打印 AST
//   print_ast(*comp_unit_node);


//   return 0;
// }

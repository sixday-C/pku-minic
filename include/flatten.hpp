#pragma once
#include <vector>
#include "ast.hpp"
#include "SymbolTable.hpp"

std::vector<int> flatten_init(
    InitValAST* init,
    const std::vector<int>& dims,
    SymbolTable& sym
);

std::vector<int> flatten_const_init(
    ConstInitValAST* init, 
    const std::vector<int>& dims, 
    SymbolTable& sym
);
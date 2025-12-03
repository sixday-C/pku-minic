#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <sstream>   
#include <list>
class BaseAST{
    public:
        virtual ~BaseAST()=default;      
};

class CompUnitAST: public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_def;
};

class FuncDefAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;
};

class FuncTypeAST: public BaseAST{
    public:
    std::string type;
    FuncTypeAST(std::string t):type(std::move(t)){}
};

class BlockAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> stmt;
};

class StmtAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> exp;
};

class ExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> unary_exp;
};

class UnaryExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> primary_exp;
    std::unique_ptr<BaseAST> unary_exp;
    char unary_op; // '+', '-', '!' or '\0' 
};

class PrimaryExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> number;
};

class NumberAST: public BaseAST{
    public:
    int value;
    NumberAST(int v):value(v){}
};    

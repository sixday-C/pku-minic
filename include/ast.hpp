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
   // public:
 //   std::unique_ptr<BaseAST> exp;
};

class ReturnStmtAST: public StmtAST{
    public:
    std::unique_ptr<BaseAST> exp;
};

class ExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> lor_exp;
};

class MulExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> unary_exp;
    std::unique_ptr<BaseAST> mul_exp;
    char mul_op; // '*', '/', '%' or '\0' 
};

class AddExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> mul_exp;
    std::unique_ptr<BaseAST> add_exp;
    char add_op; // '+', '-' or '\0' 
};

class RelExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> add_exp;
    std::unique_ptr<BaseAST> rel_exp;
    std::string rel_op; // "<", ">", "<=", ">=" or ""
};

class EqExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> rel_exp;
    std::unique_ptr<BaseAST> eq_exp;
    std::string eq_op; // "==", "!=" or ""
};

class LAndExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> eq_exp;
    std::unique_ptr<BaseAST> land_exp;
};

class LOrExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> land_exp;
    std::unique_ptr<BaseAST> lor_exp;
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

#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <sstream>   
#include <list>
#include <vector>
#include "SymbolTable.hpp"

class BaseAST{
    public:
        virtual ~BaseAST()=default; 
        virtual int evalConst(SymbolTable& sym_table) const{
            throw std::runtime_error("evalConst not implemented for this AST node");
        };
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
    std::vector<std::unique_ptr<BaseAST>> block_items;
};

class BlockItemAST: public BaseAST{
    public:
 std::unique_ptr<BaseAST> decl;
  std::unique_ptr<BaseAST> stmt;

};

class DeclAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> const_decl;
    std::unique_ptr<BaseAST> var_decl;
};

class VarDeclAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> b_type;
    std::vector<std::unique_ptr<BaseAST>> var_defs;

};

class VarDefAST: public BaseAST{
    public:
    std::string ident;
    std::unique_ptr<BaseAST> init_val;
};

class InitValAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> exp;

};


class ConstDeclAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> b_type;
    std::vector<std::unique_ptr<BaseAST>> const_defs;
};
class BTypeAST: public BaseAST{
    public:
    std::string type;
    BTypeAST(std::string t):type(std::move(t)){}
};

class ConstDefAST: public BaseAST{
    public:
    std::string ident;
    std::unique_ptr<BaseAST> const_init_val;
    
};
class ConstInitValAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> const_exp;
    int evalConst(SymbolTable& sym_table) const override {
        if (const_exp) {
            return const_exp->evalConst(sym_table);
        }
        return 0; // or some default value
    }
};

class ConstExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> exp;
    int evalConst(SymbolTable& sym_table) const override {
        if (exp) {
            return exp->evalConst(sym_table);
        }
        return 0; // or some default value
    }
};

class StmtAST: public BaseAST{
    public:
    enum class StmtType {
        Assign,      // LVal = Exp;
        Exp,         // Exp; 或 ;
        Block,       // { ... }
        Return       // return [Exp];
    };
    StmtType type;

    std::unique_ptr<BaseAST> lval;

    std::unique_ptr<BaseAST> exp;

    std::unique_ptr<BaseAST> block;



};

class ExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> lor_exp;
    int evalConst(SymbolTable& sym_table) const override {
        if (lor_exp) {
            return lor_exp->evalConst(sym_table);
        }
        return 0; // or some default value
    }
};

class MulExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> unary_exp;
    std::unique_ptr<BaseAST> mul_exp;
    char mul_op; // '*', '/', '%' or '\0' 
    int evalConst(SymbolTable& sym_table) const override {
        if (unary_exp && !mul_exp) {
            return unary_exp->evalConst(sym_table);
        }
        else if (mul_exp && unary_exp) {
            int left = mul_exp->evalConst(sym_table);
            int right = unary_exp->evalConst(sym_table);
            if (mul_op == '*') {
                return left * right;
            } else if (mul_op == '/') {
                return left / right;
            } else if (mul_op == '%') {
                return left % right;
            }
        }
        return 0; // or some default value
    }
};

class AddExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> mul_exp;
    std::unique_ptr<BaseAST> add_exp;
    char add_op; // '+', '-' or '\0' 
    int evalConst(SymbolTable& sym_table) const override {
        if (mul_exp && !add_exp) {
            return mul_exp->evalConst(sym_table);
        }
        else if (add_exp && mul_exp) {
            int left = add_exp->evalConst(sym_table);
            int right = mul_exp->evalConst(sym_table);
            if (add_op == '+') {
                return left + right;
            } else if (add_op == '-') {
                return left - right;
            }
        }
        return 0; // or some default value
    }
};

class RelExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> add_exp;
    std::unique_ptr<BaseAST> rel_exp;
    std::string rel_op; // "<", ">", "<=", ">=" or ""
    int evalConst(SymbolTable& sym_table) const override {
        if (add_exp && !rel_exp) {
            return add_exp->evalConst(sym_table);
        }
        else if (rel_exp && add_exp) {
            int left = rel_exp->evalConst(sym_table);
            int right = add_exp->evalConst(sym_table);
            if (rel_op == "<") {
                return left < right;
            } else if (rel_op == ">") {
                return left > right;
            } else if (rel_op == "<=") {
                return left <= right;
            } else if (rel_op == ">=") {
                return left >= right;
            }
        }
        return 0; // or some default value
    }
};

class EqExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> rel_exp;
    std::unique_ptr<BaseAST> eq_exp;
    std::string eq_op; // "==", "!=" or ""
    int evalConst(SymbolTable& sym_table) const override {
        if (rel_exp && !eq_exp) {
            return rel_exp->evalConst(sym_table);
        }
        else if (eq_exp && rel_exp) {
            int left = eq_exp->evalConst(sym_table);
            int right = rel_exp->evalConst(sym_table);
            if (eq_op == "==") {
                return left == right;
            } else if (eq_op == "!=") {
                return left != right;
            }
        }
        return 0; // or some default value
    }
};

class LAndExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> eq_exp;
    std::unique_ptr<BaseAST> land_exp;
    int evalConst(SymbolTable& sym_table) const override {
        if (eq_exp && !land_exp) {
            return eq_exp->evalConst(sym_table);
        }
        else if (land_exp && eq_exp) {
            int left = land_exp->evalConst(sym_table);
            int right = eq_exp->evalConst(sym_table);
            return (left != 0) && (right != 0);
        }
        return 0; // or some default value
    }
};

class LOrExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> land_exp;
    std::unique_ptr<BaseAST> lor_exp;
    int evalConst(SymbolTable& sym_table) const override {
        if (land_exp && !lor_exp) {
            return land_exp->evalConst(sym_table);
        }
        else if (lor_exp && land_exp) {
            int left = lor_exp->evalConst(sym_table);
            int right = land_exp->evalConst(sym_table);
            return (left != 0) || (right != 0);
        }
        return 0; // or some default value
    }
};

class UnaryExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> primary_exp;
    std::unique_ptr<BaseAST> unary_exp;
    char unary_op; // '+', '-', '!' or '\0'
    int evalConst(SymbolTable& sym_table) const override {
        if (primary_exp) {
            return primary_exp->evalConst(sym_table);
        }
        else if (unary_exp) {
            int operand = unary_exp->evalConst(sym_table);
            if (unary_op == '-') {
                return -operand;
            } else if (unary_op == '!') {
                return operand == 0 ? 1 : 0;
            } else if (unary_op == '+') {
                return operand;
            }
        }
        return 0; 
    }
};

//PrimaryExpAST=ExpAST | LValAST | NumberAST
class PrimaryExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> lval;
    std::unique_ptr<BaseAST> number;
    int evalConst(SymbolTable& sym_table) const override {
        if (exp) {
            return exp->evalConst(sym_table);
        }
        else if (lval) {
            return lval->evalConst(sym_table);
        }
        else if (number) {
            return number->evalConst(sym_table);
        }
        return 0; // or some default value
    }
};
class LValAST: public BaseAST{
    public:
    std::string ident;
    int evalConst(SymbolTable& sym_table) const override {
        return sym_table.lookupConst(ident);
    }
};

class NumberAST: public BaseAST{
    public:
    int value;
    NumberAST(int v):value(v){}
    int evalConst(SymbolTable& sym_table) const override {
        return value;
    }
};    

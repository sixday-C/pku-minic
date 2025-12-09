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
        virtual int evaluate(SymbolTable& sym_table) const{
            throw std::runtime_error("evaluate not implemented for this AST node");
        };
};

class CompUnitAST: public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_def;
  int evaluate(SymbolTable& sym_table) const override {
    if (func_def) {
      return func_def->evaluate(sym_table);
    }
    return 0; // or some default value
  }
};

class FuncDefAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;
    int evaluate(SymbolTable& sym_table) const override {
        if (block) {
            return block->evaluate(sym_table);
        }
        return 0; // or some default value
    }
};

class FuncTypeAST: public BaseAST{
    public:
    std::string type;
    FuncTypeAST(std::string t):type(std::move(t)){}
    int evaluate(SymbolTable& sym_table) const override {
        return 0; // or some default value
    }
};

class BlockAST: public BaseAST{
    public:
    std::vector<std::unique_ptr<BaseAST>> block_items;
    int evaluate(SymbolTable& sym_table) const override {
        int result = 0;
        for (const auto& item : block_items) {
           result = item->evaluate(sym_table);
        }
        return result;
    }
};

class BlockItemAST: public BaseAST{
    public:
 std::unique_ptr<BaseAST> decl;
  std::unique_ptr<BaseAST> stmt;
  int evaluate(SymbolTable& sym_table) const override {
    if (decl) {
      return decl->evaluate(sym_table);
    } else if (stmt) {
      return stmt->evaluate(sym_table);
    }
    return 0; // or some default value
  }

};

class DeclAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> const_decl;
    int evaluate(SymbolTable& sym_table) const override {
        if (const_decl) {
            return const_decl->evaluate(sym_table);
        }
        return 0; // or some default value
    }
};

class ConstDeclAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> b_type;
    std::vector<std::unique_ptr<BaseAST>> const_defs;
    int evaluate(SymbolTable& sym_table) const override {
       for(auto & const_def : const_defs){
        const_def->evaluate(sym_table);
       }
       return 0; // or some default value
    }
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
    int evaluate(SymbolTable& sym_table) const override {
        if (const_init_val) {
            int value = const_init_val->evaluate(sym_table);
            sym_table.insert(ident, value);
            return value;
        }
        return 0; // or some default value
    }
};
class ConstInitValAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> const_exp;
    int evaluate(SymbolTable& sym_table) const override {
        if (const_exp) {
            return const_exp->evaluate(sym_table);
        }
        return 0; // or some default value
    }
};

class ConstExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> exp;
    int evaluate(SymbolTable& sym_table) const override {
        if (exp) {
            return exp->evaluate(sym_table);
        }
        return 0; // or some default value
    }
};

class StmtAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> exp;
    int evaluate(SymbolTable& sym_table) const override {
        if (exp) {
            return exp->evaluate(sym_table);
        }
        return 0; // or some default value
    }

};

class ExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> lor_exp;
    int evaluate(SymbolTable& sym_table) const override {
        if (lor_exp) {
            return lor_exp->evaluate(sym_table);
        }
        return 0; // or some default value
    }
};

class MulExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> unary_exp;
    std::unique_ptr<BaseAST> mul_exp;
    char mul_op; // '*', '/', '%' or '\0' 
    int evaluate(SymbolTable& sym_table) const override {
        if (unary_exp && !mul_exp) {
            return unary_exp->evaluate(sym_table);
        }
        else if (mul_exp && unary_exp) {
            int left = mul_exp->evaluate(sym_table);
            int right = unary_exp->evaluate(sym_table);
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
    int evaluate(SymbolTable& sym_table) const override {
        if (mul_exp && !add_exp) {
            return mul_exp->evaluate(sym_table);
        }
        else if (add_exp && mul_exp) {
            int left = add_exp->evaluate(sym_table);
            int right = mul_exp->evaluate(sym_table);
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
    int evaluate(SymbolTable& sym_table) const override {
        if (add_exp && !rel_exp) {
            return add_exp->evaluate(sym_table);
        }
        else if (rel_exp && add_exp) {
            int left = rel_exp->evaluate(sym_table);
            int right = add_exp->evaluate(sym_table);
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
    int evaluate(SymbolTable& sym_table) const override {
        if (rel_exp && !eq_exp) {
            return rel_exp->evaluate(sym_table);
        }
        else if (eq_exp && rel_exp) {
            int left = eq_exp->evaluate(sym_table);
            int right = rel_exp->evaluate(sym_table);
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
    int evaluate(SymbolTable& sym_table) const override {
        if (eq_exp && !land_exp) {
            return eq_exp->evaluate(sym_table);
        }
        else if (land_exp && eq_exp) {
            int left = land_exp->evaluate(sym_table);
            int right = eq_exp->evaluate(sym_table);
            return (left != 0) && (right != 0);
        }
        return 0; // or some default value
    }
};

class LOrExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> land_exp;
    std::unique_ptr<BaseAST> lor_exp;
    int evaluate(SymbolTable& sym_table) const override {
        if (land_exp && !lor_exp) {
            return land_exp->evaluate(sym_table);
        }
        else if (lor_exp && land_exp) {
            int left = lor_exp->evaluate(sym_table);
            int right = land_exp->evaluate(sym_table);
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
    int evaluate(SymbolTable& sym_table) const override {
        if (primary_exp) {
            return primary_exp->evaluate(sym_table);
        }
        else if (unary_exp) {
            int operand = unary_exp->evaluate(sym_table);
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

class PrimaryExpAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> lval;
    std::unique_ptr<BaseAST> number;
    int evaluate(SymbolTable& sym_table) const override {
        if (exp) {
            return exp->evaluate(sym_table);
        }
        else if (lval) {
            return lval->evaluate(sym_table);
        }
        else if (number) {
            return number->evaluate(sym_table);
        }
        return 0; // or some default value
    }
};
class LValAST: public BaseAST{
    public:
    std::string ident;
    int evaluate(SymbolTable& sym_table) const override {
        return sym_table.lookup(ident);
    }
};

class NumberAST: public BaseAST{
    public:
    int value;
    NumberAST(int v):value(v){}
    int evaluate(SymbolTable& sym_table) const override {
        return value;
    }
};    

#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <sstream>   


// 所有 AST 的基类
struct IRContext { int id = 0; };
static IRContext g_ir;
static std::string g_last_val;   


class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void Dump() const = 0;
  virtual void IR(std::ostream &out) const  = 0; 
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_def;

    void Dump() const override;
    void IR(std::ostream &out) const override;
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;

    void Dump() const override;
    void IR(std::ostream &out) const override;
};

class BlockAST: public BaseAST{
    public:
    std::unique_ptr<BaseAST> stmt;

    void Dump() const override;
    void IR(std::ostream &out) const override;
};

class FuncTypeAST : public BaseAST{
    public:
    std::string type;
    FuncTypeAST(std::string t) : type(std::move(t)) {}

    void Dump() const override;
    void IR(std::ostream &out) const override;
};

class NumberAST:public BaseAST{
    public:
    int value;
    NumberAST(int v) : value(v) {}

    void Dump() const override;
    void IR(std::ostream &out) const override;
};

class StmtAST :public BaseAST{
    public:
    std::unique_ptr<BaseAST> Exp;
    ~StmtAST() override = default;

    void Dump() const override;
    void IR(std::ostream &out) const override;
};

class ExpAST :public BaseAST{
    public:
    std::unique_ptr<BaseAST> UnaryExp;
    ~ExpAST() override = default;

    void Dump() const override;
    void IR(std::ostream &out) const override;
};

class PrimaryExpAST :public BaseAST{
    public:
    std::unique_ptr<BaseAST> Exp; // "(" Exp ")"
    std::unique_ptr<BaseAST> Number; // Number
    ~PrimaryExpAST() override = default;

    void Dump() const override;
    void IR(std::ostream &out) const override;
};

class UnaryExpAST :public BaseAST{
    public:
    std::unique_ptr<BaseAST> PrimaryExp; // PrimaryExp
    char UnaryOp; // UnaryOp
    std::unique_ptr<BaseAST> UnaryExp; // UnaryExp
    ~UnaryExpAST() override = default;

    void Dump() const override;
    void IR(std::ostream &out) const override;
};



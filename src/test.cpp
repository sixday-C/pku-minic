#pragma once
#include <memory>
#include <vector>
#include <string>
#include <ostream>
using namespace std;
class Value {
public:
    virtual ~Value() = default;
    virtual void dump(std::ostream& os) const = 0;
};

class Integer : public Value {
public:
    int value;
    explicit Integer(int v) : value(v) {}
    void dump(std::ostream& os) const override{
        os << value;
    };
};

class Variable : public Value {
public:
    std::string name;
    explicit Variable(const std::string& n) : name(n) {}
    void dump(std::ostream& os) const override {
        os << name;
    }
};

class Binary : public Value {
public:
    enum Op{
        EQ,   
        SUB   
    };
    Op op;
    unique_ptr<Value> lhs;  
    unique_ptr<Value> rhs;             
    unique_ptr<Value> Var;
    Binary(Op operation, unique_ptr<Value> l, unique_ptr<Value> r)
        : op(operation), lhs(std::move(l)), rhs(std::move(r)) {}
    void dump(std::ostream& os) const override;//cpp实现
};

class Return : public Value {
public:
    unique_ptr<Value> value;      

    explicit Return(unique_ptr<Value> val) : value(std::move(val)) {}
    
    Return() = default;
    void dump(std::ostream& os) const override;
};

class BasicBlock {
public:
    string name;                             
    vector<unique_ptr<Value>> insts;     
    
    explicit BasicBlock(const string& n) : name(n) {}
    
    void add(unique_ptr<Value> inst);         
    void dump(std::ostream& os) const;             
};

class Function {
public:
    string name;                                   
    vector<unique_ptr<BasicBlock>> blocks;     
    
    explicit Function(const string& n) : name(n) {}
    
    void add(unique_ptr<BasicBlock> bb);            
    void dump(std::ostream& os) const;                   
};

class Program {
public:
    vector<unique_ptr<Function>> funcs;       
    
    void add(unique_ptr<Function> func);            
    void dump(std::ostream& os) const;                   
};

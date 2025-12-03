#pragma once
#include <memory>
#include <vector>
#include <string>
#include <ostream>
#include <list>

/*
Program
Function
BasicBlock
Types
    -i32

Value
    - Binary
*/

//类型
enum class Type{
    Int32,
    Void,
    Label
};

//操作类型
enum class OpType{
    Ret,
    Eq,
    Add,
    Sub,
};

inline std::string opName(OpType op) {
    switch (op) {
        case OpType::Eq: return "eq";
        case OpType::Add: return "add";
        case OpType::Sub: return "sub";
        case OpType::Ret: return "ret";
        default: return "unknown";
    }
}
class Value {
public:
    virtual ~Value() = default;
    Type type;
    std::string name; 
    virtual std::string toString() const = 0;
};

class Integer: public Value {
public:
    int value;

    Integer(int v): value(v) {
        type = Type::Int32;
        name = std::to_string(v);
    }

    std::string toString() const override {
        return name;
    }
};

class Instruction: public Value {
public:
    OpType op;
    Instruction(OpType operation, Type t, const std::string& n):
        op(operation)
    {
        type = t;
        name = n;
    }
};

class Binary: public Instruction {
public:
    Value* lhs;
    Value* rhs;
    Binary(OpType operation, Value* l, Value* r, const std::string& n)
        : Instruction(operation, Type::Int32, n), lhs(l), rhs(r) {}

    std::string toString() const override {
       return name + " = " + opName(op) + " " + lhs->name + ", " + rhs->name;
    }
};

class ReturnInst : public Instruction {
public:
    Value* retValue;
    ReturnInst(Value* v) 
        : Instruction(OpType::Ret, Type::Void, ""), retValue(v) {}

    std::string toString() const override {
        return "ret " + retValue->name;
    }
};

class BasicBlock : public Value {
public:
    std::list<std::unique_ptr<Instruction>> insts;
    BasicBlock(const std::string &n) {name=n;type=Type::Label;}
    void addInst(Instruction* inst) {
        insts.push_back(std::unique_ptr<Instruction>(inst));
    }
    std::string toString() const override {
        std::string result = name + ":\n";
        for (const auto& inst : insts) {
            result += "  " + inst->toString() + "\n";
        }
        return result;
    }
};
class Function : public Value {
public:
    std::list<std::unique_ptr<BasicBlock>> blocks;
    Function(const std::string &n) {name=n;type=Type::Int32;}
    void addBlock(BasicBlock* block) {
        blocks.push_back(std::unique_ptr<BasicBlock>(block));
    }
    std::string toString() const override {
        std::string result = "fun " + name + "(): i32 {\n";
        for (const auto& block : blocks) {
            result += block->toString();
        }
        result += "}\n";
        return result;
    }
};
class Program {
public:
    std::list<std::unique_ptr<Function>> funcs;
};
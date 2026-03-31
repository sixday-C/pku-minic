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
    - ReturnInst
    - AllocInst
    - StoreInst
    - LoadInst
*/

class BasicBlock;
class Instruction;
class Value;
//类型
enum class Type{
    Int32,
    Void,
    Label,
    Pointer
};

//操作类型
enum class OpType{
    Ret,
    Eq,
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Lt, //<
    Gt, //>
    Le, //<=
    Ge, //>=
    Ne, //!=
    AND, // &   用来拼接&&
    OR, // |    用来拼接||
    Alloc,
    Store,
    Load,
    Br,
    Jump,
    Call//函数调用
};

inline std::string opName(OpType op) {
    switch (op) {
        case OpType::Eq: return "eq";
        case OpType::Add: return "add";
        case OpType::Sub: return "sub";
        case OpType::Ret: return "ret";
        case OpType::Mul: return "mul";
        case OpType::Div: return "div";
        case OpType::Mod: return "mod";
        case OpType::Lt: return "lt";
        case OpType::Gt: return "gt";
        case OpType::Le: return "le";
        case OpType::Ge: return "ge";
        case OpType::Ne: return "ne";
        case OpType::AND: return "and";
        case OpType::OR: return "or";
        case OpType::Alloc: return "alloc";
        case OpType::Store: return "store";
        case OpType::Load: return "load";
        case OpType::Br: return "br";
        case OpType::Jump: return "jump";
        case OpType::Call: return "call";
        default: return "unknown";
    }
}
class Value {
public:
    virtual ~Value() = default;
    Type type;
    std::string name; 
    virtual std::string toString() const = 0;
    virtual bool isGlobal() const { return false; }
};

//全局变量
class GlobalAlloc : public Value {
public:
    int value; // 仅支持整数常量
    GlobalAlloc(const std::string& n, int v) : value(v) {
        name = n;
        type = Type::Int32;
    }
    std::string toString() const override {
        if (value == 0) {
            return "global " + name + " = alloc i32, zeroinit\n";
        }
        return "global " + name + " = alloc i32, " + std::to_string(value) + "\n";
    }
    bool isGlobal() const override { return true; }
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

class Parameter : public Value {
public:
    Parameter(const std::string& n) { name = n; type = Type::Int32; }
    std::string toString() const override { return name; }
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

class BranchInst : public Instruction {
public:
    Value* condition;
    BasicBlock* thenBlock;
    BasicBlock* elseBlock;

    BranchInst(Value* cond, BasicBlock* thenB, BasicBlock* elseB)
        : Instruction(OpType::Br, Type::Void, ""), condition(cond), thenBlock(thenB), elseBlock(elseB) {}
    std::string toString() const override ;
};

class JumpInst : public Instruction {
public:
    BasicBlock* targetBlock;
    JumpInst(BasicBlock* target)
        : Instruction(OpType::Jump, Type::Void, ""), targetBlock(target) {}
    std::string toString() const override ;
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
    ReturnInst(Value* v = nullptr) 
        : Instruction(OpType::Ret, Type::Void, ""), retValue(v) {}

    std::string toString() const override {
        if (!retValue) return "ret";
        return "ret " + retValue->name;
    }
};

class AllocInst : public Instruction {
public:
    AllocInst(const std::string& n)
        : Instruction(OpType::Alloc, Type::Int32, n) {        
        } 
    std::string toString() const override {
        return name + " = alloc i32";
    }
}; 
class StoreInst : public Instruction {
public:
    Value* value;
    Value* address;
    StoreInst(Value* val, Value* addr)
        : Instruction(OpType::Store, Type::Void, ""), value(val), address(addr) {
        } 
    std::string toString() const override {
        return "store " + value->name + ", " + address->name;
    }
};
class LoadInst : public Instruction {
public:
    Value* address;
    LoadInst(Value* addr, const std::string& n)
        : Instruction(OpType::Load, Type::Int32, n), address(addr) {
        } 
    std::string toString() const override {
        return name + " = load " + address->name;
    }
};
class CallInst : public Instruction {
public:
    std::string funcName;
    std::vector<Value*> args;
    CallInst(const std::string& fName, const std::vector<Value*>& arguments, Type retType, const std::string& n)
        : Instruction(OpType::Call, retType, n), funcName(fName), args(arguments) {}

    std::string toString() const override {
        std::string res = (type == Type::Void ? "" : name + " = ") + "call " + funcName + "(";
        for (size_t i = 0; i < args.size(); ++i) {
            res += args[i]->name + (i == args.size() - 1 ? "" : ", ");
        }
        return res + ")";
    }
};

class BasicBlock : public Value {
public:
    std::list<std::unique_ptr<Instruction>> insts;
    std::list<std::unique_ptr<Value>> values; 
    BasicBlock(const std::string &n) {name=n;type=Type::Label;}
    void addInst(Instruction* inst) {
        insts.push_back(std::unique_ptr<Instruction>(inst));
    }
    void addValue(Value* val) {
        values.push_back(std::unique_ptr<Value>(val));
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
    std::vector<std::pair<std::string, Type>> params; // 存储参数名和类型
    Type retType;

    Function(const std::string &n, Type rt) : retType(rt) {
        name = n;
        type = rt;
    }

    void addBlock(BasicBlock* block) {
        blocks.push_back(std::unique_ptr<BasicBlock>(block));
    }

    std::string toString() const override {
        // 生成形如: fun @foo(%a: i32, %b: i32): i32 {
        std::string result = "fun " + name + "(";
        for (size_t i = 0; i < params.size(); ++i) {
            result += params[i].first + ": i32" + (i == params.size() - 1 ? "" : ", ");
        }
        result += ")";  
        if(retType != Type::Void){
            result += ": i32";
        }
        result += " {\n";
        for (const auto& block : blocks) {
            result += block->toString();
        }
        result += "}\n";
        return result;
    }
};

class Program {
public:
    struct DeclInfo {
        std::string name;
        Type retType;
        std::vector<Type> paramTypes;
    };
    std::vector<DeclInfo> decls;

    std::list<std::unique_ptr<Function>> funcs;
    std::list<std::unique_ptr<Value>> globalValues; // 用于存储全局变量和全局常量
    void toString(std::ostream& os) const {
        for (const auto& d : decls) {
            os << "decl " << d.name << "(";
            for (size_t i = 0; i < d.paramTypes.size(); ++i) {
                switch(d.paramTypes[i]){
                    case Type::Int32: os << "i32"; break;
                    case Type::Pointer: os << "*i32"; break;
                    default: os << "unknown"; break;
                }
                if (i != d.paramTypes.size() - 1) os << ", ";
            }
            os << ")";
            if (d.retType != Type::Void) os << ": i32";
            os << "\n";
        }
        if (!decls.empty()) os << "\n";
        for (const auto& def : globalValues) {
            os << def->toString();
        }
        if (!globalValues.empty() && !funcs.empty()) {
            os << "\n";
        }
        for (const auto& func : funcs) {
            os << func->toString();
        }
    }
};



inline std::string BranchInst::toString() const {
    return "br " + condition->name + ", " + thenBlock->name + ", " + elseBlock->name;
}

inline std::string JumpInst::toString() const {
    return "jump " + targetBlock->name;
}
#pragma once
#include <memory>
#include <vector>
#include <string>
#include <ostream>

// 基础 Value 类 - 所有 IR 值的基类
class Value {
public:
    virtual ~Value() = default;
    virtual void dump(std::ostream& os) const = 0;
};

// 整数常量类 - 用于表示字面量
class Integer : public Value {
public:
    int value;
    
    explicit Integer(int v) : value(v) {}
    
    void dump(std::ostream& os) const override;
};

// 常量整数类 - 与 Integer 功能类似，用于 RvGen
class ConstInt : public Value {
public:
    int v;
    
    explicit ConstInt(int val) : v(val) {}
    
    void dump(std::ostream& os) const override {
        os << v;
    }
};

// 二元运算指令
class Binary : public Value {
public:
    enum OpType {
        EQ,   // 相等比较
        SUB   // 减法
    };
    
    OpType op;
    std::unique_ptr<Value> lhs;  // 左操作数
    std::unique_ptr<Value> rhs;  // 右操作数
    std::string name;            // 结果变量名
    
    Binary(OpType operation, std::unique_ptr<Value> l, std::unique_ptr<Value> r)
        : op(operation), lhs(std::move(l)), rhs(std::move(r)) {}
    
    void dump(std::ostream& os) const override;
};

// 返回指令
class Return : public Value {
public:
    std::unique_ptr<Value> value;  // 返回值（可选）
    std::string temp_name;         // 临时变量名（可选）
    
    // 构造函数：返回常量值
    explicit Return(std::unique_ptr<Value> val) : value(std::move(val)) {}
    
    // 构造函数：返回临时变量
    explicit Return(const std::string& temp) : temp_name(temp) {}
    
    // 构造函数：无返回值
    Return() = default;
    
    void dump(std::ostream& os) const override;
};

// 基本块 - 包含一系列指令
class BasicBlock {
public:
    std::string name;                              // 基本块名称
    std::vector<std::unique_ptr<Value>> insts;     // 指令序列
    
    explicit BasicBlock(const std::string& n) : name(n) {}
    
    void add(std::unique_ptr<Value> inst);         // 添加指令
    void dump(std::ostream& os) const;             // 输出基本块
};

// 函数 - 包含多个基本块
class Function {
public:
    std::string name;                                    // 函数名
    std::vector<std::unique_ptr<BasicBlock>> blocks;     // 基本块序列
    
    explicit Function(const std::string& n) : name(n) {}
    
    void add(std::unique_ptr<BasicBlock> bb);            // 添加基本块
    void dump(std::ostream& os) const;                   // 输出函数
};

// 程序 - 包含多个函数
class Program {
public:
    std::vector<std::unique_ptr<Function>> funcs;        // 函数序列
    
    void add(std::unique_ptr<Function> func);            // 添加函数
    void dump(std::ostream& os) const;                   // 输出程序
};

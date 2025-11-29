#include "../include/ir.hpp"

void Integer::dump(std::ostream& os) const {
    os << value;
}

// 二元操作：输出 "name = op lhs, rhs"
void Binary::dump(std::ostream& os) const {
    os << name << " = ";
    
    // 简化：只处理我们需要的操作符
    if (op == EQ) os << "eq";
    else if (op == SUB) os << "sub";
    
    os << " ";
    lhs->dump(os);
    os << ", ";
    rhs->dump(os);
}

// return 指令
void Return::dump(std::ostream& os) const {
    os << "ret";
    if (value) {
        os << " ";
        value->dump(os);
    } else if (!temp_name.empty()) {
        os << " " << temp_name;
    }
}

// 基本块：添加指令
void BasicBlock::add(std::unique_ptr<Value> v) {
    insts.push_back(std::move(v));
}

// 基本块：输出
void BasicBlock::dump(std::ostream& os) const {
    os << "%" << name << ":\n";
    for (const auto& inst : insts) {
        os << "  ";
        inst->dump(os);
        os << "\n";
    }
}

// 函数：添加基本块
void Function::add(std::unique_ptr<BasicBlock> bb) {
    blocks.push_back(std::move(bb));
}

// 函数：输出
void Function::dump(std::ostream& os) const {
    os << "fun @" << name << "(): i32 {\n";
    for (const auto& bb : blocks) {
        bb->dump(os);
    }
    os << "}\n";
}

// 程序：添加函数
void Program::add(std::unique_ptr<Function> f) {
    funcs.push_back(std::move(f));
}

// 程序：输出
void Program::dump(std::ostream& os) const {
    for (const auto& f : funcs) {
        f->dump(os);
    }
}

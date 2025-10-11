#include "../include/ast.hpp"

void CompUnitAST::Dump() const {
    std::cout << "CompUnitAST { \n";
    func_def->Dump();
    std::cout << " }\n";
}

void CompUnitAST::IR(std::ostream &out) const {
    func_def->IR(out); // 把同一流传给子节点
}

void FuncDefAST::Dump() const {
    std::cout << "FuncDefAST { \n";
    func_type->Dump();
    std::cout << ", " << ident << ", ";
    block->Dump();
    std::cout << " }\n";
}

void FuncDefAST::IR(std::ostream &out) const {
    g_ir.id = 0;
    out << "fun @" << ident << "(): ";
    func_type->IR(out);
    block->IR(out);
}

void BlockAST::Dump() const {
    std::cout << "BlockAST:{\n";
    stmt->Dump();
    std::cout << "}\n";
}

void BlockAST::IR(std::ostream &out) const {
    out << " {\n";
    out << "%entry:\n";
    stmt->IR(out);
    out << "}\n";
}

void FuncTypeAST::Dump() const {
    std::cout << "FuncTypeAST:";
    std::cout << type << "\n";
}

void FuncTypeAST::IR(std::ostream &out) const {
    out << "i32";
}

void NumberAST::Dump() const {
    std::cout << "NumberAST : ";
    std::cout << value;
}

void NumberAST::IR(std::ostream &out) const {
    out << value;
}

void StmtAST::Dump() const {
    std::cout << " StmtAST :"; 
    std::cout << "return "; 
    Exp->Dump();
    std::cout << ";\n";
}

void StmtAST::IR(std::ostream &out) const {
    Exp->IR(out);
    out << "  ret " << g_last_val << "\n";
}

void ExpAST::Dump() const {
    std::cout << " ExpAST :\n"; 
    UnaryExp->Dump();
}

void ExpAST::IR(std::ostream &out) const {
    UnaryExp->IR(out);
}

void PrimaryExpAST::Dump() const {
    std::cout << " PrimaryExpAST :"; 
    if(Exp) Exp->Dump();
    if(Number) Number->Dump();
}

void PrimaryExpAST::IR(std::ostream &out) const {
    if (Number) { 
        std::ostringstream ss;
        Number->IR(ss); 
        g_last_val = ss.str(); // 保存常量值
    }
    if(Exp){
        Exp->IR(out); 
    }
}



void UnaryExpAST::Dump() const {
    std::cout << " UnaryExpAST :"; 
    if(PrimaryExp) PrimaryExp->Dump();
    if(UnaryOp != '\0') std::cout << " UnaryOp: " << UnaryOp << " ";
    if(UnaryExp) UnaryExp->Dump();
}

void UnaryExpAST::IR(std::ostream &out) const {
    if (PrimaryExp) { PrimaryExp->IR(out); return; } 
    if (UnaryOp != '\0' && UnaryExp) {
        // 1) 先计算右侧，结束后 g_last_val=操作数文本
        UnaryExp->IR(out);
        const std::string rhs = g_last_val;

        // 2) 根据操作符生成/不生成指令，并更新 g_last_val
        if (UnaryOp == '!') {
            const std::string t = new_tmp();
            out << "  " << t << " = eq " << rhs << ", 0\n";
            g_last_val = t;
            return;
        }
        if (UnaryOp == '-') {
            const std::string t = new_tmp();
            out << "  " << t << " = sub 0, " << rhs << "\n";
            g_last_val = t;
            return;
        }
        if(UnaryOp == '+'){}
    }
}

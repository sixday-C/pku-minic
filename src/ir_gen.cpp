#include "../include/ir_gen.hpp"
#include "../include/ir.hpp"
#include "../include/ast.hpp"
#include <cassert>

// 简化版 IR 生成器，类似原来的 AST 方式
std::unique_ptr<Program> IRGenerator::Generate(const CompUnitAST& ast) {
    program = std::make_unique<Program>();
    temp_counter = 0;
    
    // 创建函数
    auto* func_def = static_cast<const FuncDefAST*>(ast.func_def.get());
    auto func = std::make_unique<Function>(func_def->ident);
    auto bb = std::make_unique<BasicBlock>("entry");
    
    current_block = bb.get();
    
    // 处理 return 语句 - 找到表达式
    auto* block = static_cast<const BlockAST*>(func_def->block.get());
    auto* stmt = static_cast<const StmtAST*>(block->stmt.get());
    
    // 生成表达式 IR，last_value 保存最终结果
    generate_expr(*stmt->Exp);
    
    
    // 防止程序崩溃?我也不知道ai写的这段代码有任何有用的意思没有
    // 创建 return 指令
    if (last_value.empty()) {
        if (auto* exp = dynamic_cast<const ExpAST*>(stmt->Exp.get())) {
            if (auto* unary = dynamic_cast<const UnaryExpAST*>(exp->UnaryExp.get())) {
                if (auto* primary = dynamic_cast<const PrimaryExpAST*>(unary->PrimaryExp.get())) {
                    if (auto* num = dynamic_cast<const NumberAST*>(primary->Number.get())) {
                        auto ret = std::make_unique<Return>(std::make_unique<Integer>(num->value));
                        current_block->add(std::move(ret));
                    }
                }
            }
        }
    } else {
        // 这是临时变量，直接传递变量名
        auto ret = std::make_unique<Return>(last_value);
        current_block->add(std::move(ret));
    }
    
    func->add(std::move(bb));
    program->add(std::move(func));
    return std::move(program);
}

// 生成表达式 IR - 超简单版本，类似原来的 AST 方式
void IRGenerator::generate_expr(const BaseAST& expr) {
    // NumberAST - 常量
    if (auto* num = dynamic_cast<const NumberAST*>(&expr)) {
        last_value = std::to_string(num->value);
        is_constant = true;  // 标记为常量
        return;
    }
    
    // ExpAST - 直接处理 UnaryExp
    if (auto* exp = dynamic_cast<const ExpAST*>(&expr)) {
        generate_expr(*exp->UnaryExp);
        return;
    }
    
    // PrimaryExpAST - 处理 Number 或 (Exp)
    if (auto* primary = dynamic_cast<const PrimaryExpAST*>(&expr)) {
        if (primary->Number) {
            generate_expr(*primary->Number);
            return;
        }
        if (primary->Exp) {
            generate_expr(*primary->Exp);
            return;
        }
    }
    
    // UnaryExpAST - 核心处理
    if (auto* unary = dynamic_cast<const UnaryExpAST*>(&expr)) {
        if (unary->PrimaryExp) {
            generate_expr(*unary->PrimaryExp);
            return;
        }
        
        if (unary->UnaryOp != '\0' && unary->UnaryExp) {
            // 先计算右操作数
            generate_expr(*unary->UnaryExp);
            std::string rhs = last_value;
            bool rhs_is_const = is_constant;
            
            if (unary->UnaryOp == '!') {
                std::string temp = new_temp();
                std::unique_ptr<Value> lhs_val;
                if (rhs_is_const) {
                    lhs_val = std::make_unique<Integer>(std::stoi(rhs));
                } else {
                    lhs_val = std::make_unique<Variable>(rhs);
                }
                auto binary = std::make_unique<Binary>(Binary::EQ,
                    std::move(lhs_val),
                    std::make_unique<Integer>(0));
                binary->name = temp;
                current_block->add(std::move(binary));
                last_value = temp;
                is_constant = false;  // 结果是临时变量
                return;
            }
            
            if (unary->UnaryOp == '-') {
                std::string temp = new_temp();
                std::unique_ptr<Value> rhs_val;
                if (rhs_is_const) {
                    rhs_val = std::make_unique<Integer>(std::stoi(rhs));
                } else {
                    rhs_val = std::make_unique<Variable>(rhs);
                }
                auto binary = std::make_unique<Binary>(Binary::SUB,
                    std::make_unique<Integer>(0),
                    std::move(rhs_val));
                binary->name = temp;
                current_block->add(std::move(binary));
                last_value = temp;
                is_constant = false;  // 结果是临时变量
                return;
            }
            
            if (unary->UnaryOp == '+') {
                // +x = x，last_value 和 is_constant 保持不变
                return;
            }
        }
    }
}

// 生成临时变量名
std::string IRGenerator::new_temp() {
    return "%" + std::to_string(temp_counter++);
}

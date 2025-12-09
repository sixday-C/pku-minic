#pragma once
#include "ast.hpp" 
#include "ir.hpp"  
#include <cassert> 
#include "SymbolTable.hpp"
class IRGenerator {
public:
    std::unique_ptr<Program> program;
    SymbolTable sym_table;
private:
    Function* currentFunc=nullptr;
    BasicBlock* currentBlock=nullptr;
    Value* lastVal = nullptr;

    int tempCounter=0;

    std::string newTemp() {
        return "%" + std::to_string(tempCounter++);
    }

public:
    IRGenerator() : program(std::make_unique<Program>()) {}

    std::unique_ptr<Program> getProgram() {
        return std::move(program);
    }

void visit(CompUnitAST* ast){
    ast->evaluate(sym_table);

    auto funcDef = static_cast<FuncDefAST*>(ast->func_def.get());
    visit(*funcDef);
}

void visit(FuncDefAST& ast) {
    tempCounter=0;
    auto func=std::make_unique<Function>("@"+ast.ident);
    currentFunc=func.get();
    auto block=std::make_unique<BasicBlock>("%entry");
    currentBlock=block.get();
    func->addBlock(block.release());
    auto blockNode = static_cast<BlockAST*>(ast.block.get());
    visit(*blockNode);
    program->funcs.push_back(std::move(func));
    }
    
    void visit(BlockAST& ast) {
        for(auto& item : ast.block_items){
            auto block_item = static_cast<BlockItemAST*>(item.get());
            if (block_item->decl) {
            // 不生成 IR（常量已在 evaluate 阶段处理）
            continue;
        }
        else if (block_item->stmt) {
            auto stmt = static_cast<StmtAST*>(block_item->stmt.get());
            visit(*stmt);
        }  
    }
    } 
    void visit(StmtAST& ast) {
        if(ast.exp){
            auto exp= static_cast<ExpAST*>(ast.exp.get());
            visit(*exp);
            auto retInst = new ReturnInst(lastVal);
            currentBlock->addInst(retInst);
        }
    }
    
    void visit(ExpAST& ast) {
        if(ast.lor_exp){
            auto lorExp= static_cast<LOrExpAST*>(ast.lor_exp.get());
            visit(*lorExp);
        }
    }
    
    void visit(RelExpAST & ast){
        if(ast.add_exp && !ast.rel_exp){
            auto addExp= static_cast<AddExpAST*>(ast.add_exp.get());
            visit(*addExp);
            return;
        }
        else if(ast.rel_exp && ast.add_exp){
            visit(*static_cast<RelExpAST*>(ast.rel_exp.get()));
            Value* left = lastVal;
            visit(*static_cast<AddExpAST*>(ast.add_exp.get()));
            Value* right = lastVal;

            Instruction* inst = nullptr;
            if (ast.rel_op == "<") {
                inst = new Binary(OpType::Lt, left, right, newTemp());
            } 
            else if (ast.rel_op == ">") {
                inst = new Binary(OpType::Gt, left, right, newTemp());
            }
            else if (ast.rel_op == "<=") {
                inst = new Binary(OpType::Le, left, right, newTemp());
            }
            else if (ast.rel_op == ">=") {
                inst = new Binary(OpType::Ge, left, right, newTemp());
            }

            if (inst) {
                currentBlock->addInst(inst);
                lastVal = inst; 
            }
        }
    }
    void visit(EqExpAST & ast){
        if(ast.rel_exp && !ast.eq_exp){
            auto relExp= static_cast<RelExpAST*>(ast.rel_exp.get());
            visit(*relExp);
        }
        else if(ast.eq_exp && ast.rel_exp){
            visit(*static_cast<EqExpAST*>(ast.eq_exp.get()));
            Value* left = lastVal;
            visit(*static_cast<RelExpAST*>(ast.rel_exp.get()));
            Value* right = lastVal;

            Instruction* inst = nullptr;
            if (ast.eq_op == "==") {
                inst = new Binary(OpType::Eq, left, right, newTemp());
            } 
            else if (ast.eq_op == "!=") {
                inst = new Binary(OpType::Ne, left, right, newTemp());
            }

            if (inst) {
                currentBlock->addInst(inst);
                lastVal = inst; 
            }
        }
    }
    void visit(LAndExpAST& ast){
        if(ast.eq_exp && !ast.land_exp){
            //拼接逻辑&&
            auto eqExp= static_cast<EqExpAST*>(ast.eq_exp.get());
            visit(*eqExp);
        }
        else if(ast.land_exp && ast.eq_exp){
            //左边
            visit(*static_cast<LAndExpAST*>(ast.land_exp.get()));
            Value* left = lastVal;

            //%1 = ne a, 0
            auto zero1 = new Integer(0);
            currentBlock->addValue(zero1);
            auto cmp1 = new Binary(OpType::Ne, left, zero1, newTemp());
            currentBlock->addInst(cmp1);

            //右边
            visit(*static_cast<EqExpAST*>(ast.eq_exp.get()));
            Value* right = lastVal;

            //%2 = ne b, 0
            auto zero2 = new Integer(0);
            currentBlock->addValue(zero2);
            auto cmp2 = new Binary(OpType::Ne, right, zero2, newTemp());
            currentBlock->addInst(cmp2);

            auto inst = new Binary(OpType::AND, cmp1, cmp2, newTemp());
                currentBlock->addInst(inst);
                lastVal = inst; 
            }
        }
    
    void visit(LOrExpAST& ast){
        if(ast.land_exp && !ast.lor_exp){
            auto landExp= static_cast<LAndExpAST*>(ast.land_exp.get());
            visit(*landExp);
        }
        else if(ast.lor_exp && ast.land_exp){
            //左边
            visit(*static_cast<LOrExpAST*>(ast.lor_exp.get()));
            Value* left = lastVal;
            //%1 = ne a, 0
            auto zero1 = new Integer(0);
            currentBlock->addValue(zero1);
            auto cmp1 = new Binary(OpType::Ne, left, zero1, newTemp());
            currentBlock->addInst(cmp1);
            //右边
            visit(*static_cast<LAndExpAST*>(ast.land_exp.get()));
            Value* right = lastVal;
            //%2 = ne b, 0
            auto zero2 = new Integer(0);
            currentBlock->addValue(zero2);
            auto cmp2 = new Binary(OpType::Ne, right, zero2, newTemp());
            currentBlock->addInst(cmp2);
            
            auto inst = new Binary(OpType::OR, cmp1, cmp2, newTemp());
                currentBlock->addInst(inst);
                lastVal = inst;
        }
    }

    void visit(AddExpAST& ast){
        if(ast.mul_exp && !ast.add_exp){
            auto mulExp= static_cast<MulExpAST*>(ast.mul_exp.get());
            visit(*mulExp);
            return;
        }
        else if(ast.add_exp && ast.mul_exp){
            visit(*static_cast<AddExpAST*>(ast.add_exp.get()));
            Value* left = lastVal;
            visit(*static_cast<MulExpAST*>(ast.mul_exp.get()));
            Value* right = lastVal;

            Instruction* inst = nullptr;
            if (ast.add_op == '+') {
                inst = new Binary(OpType::Add, left, right, newTemp());
            } 
            else if (ast.add_op == '-') {
                inst = new Binary(OpType::Sub, left, right, newTemp());
            }

            if (inst) {
                currentBlock->addInst(inst);
                lastVal = inst; 
            }
        }
    }
    void visit(MulExpAST& ast){
        if(ast.unary_exp && !ast.mul_exp){
            auto unaryExp= static_cast<UnaryExpAST*>(ast.unary_exp.get());
            visit(*unaryExp);
            return;
        }
        else if(ast.mul_exp && ast.unary_exp){
            visit(*static_cast<MulExpAST*>(ast.mul_exp.get()));
            Value* left = lastVal;
            visit(*static_cast<UnaryExpAST*>(ast.unary_exp.get()));
            Value* right = lastVal;

            Instruction* inst = nullptr;
            if (ast.mul_op == '*') {
                inst = new Binary(OpType::Mul, left, right, newTemp());
            } 
            else if (ast.mul_op == '/') {
                inst = new Binary(OpType::Div, left, right, newTemp());
            }
            else if (ast.mul_op == '%') {
                inst = new Binary(OpType::Mod, left, right, newTemp());
            }

            if (inst) {
                currentBlock->addInst(inst);
                lastVal = inst; 
            }
        }
    }
    void visit(UnaryExpAST& ast) {
        if(ast.primary_exp){
            auto primaryExp= static_cast<PrimaryExpAST*>(ast.primary_exp.get());
            visit(*primaryExp);
            return;
        }
        else if(ast.unary_exp){
            char op = ast.unary_op;
            auto innerExp = static_cast<UnaryExpAST*>(ast.unary_exp.get());
            visit(*innerExp); 

            Value* operand = lastVal; 
            Instruction* inst = nullptr;

            if (op == '-') {
                auto zero = new Integer(0);
                currentBlock->addValue(zero);
                inst = new Binary(OpType::Sub, zero, operand, newTemp());
            } 
            else if (op == '!') {
                auto zero = new Integer(0);
                currentBlock->addValue(zero);
                inst = new Binary(OpType::Eq, operand, zero, newTemp());
            }
            else if (op == '+') {
                return;
            }

            if (inst) {
                currentBlock->addInst(inst);
                lastVal = inst; 
            }
        }
    }
    void visit(PrimaryExpAST& ast) {
        if(ast.number){
            auto num= static_cast<NumberAST*>(ast.number.get());
            visit(*num);
        }
        else if (ast.exp) {
            auto exp = static_cast<ExpAST*>(ast.exp.get());
            visit(*exp);
        }
        else if (ast.lval){
            auto lval = static_cast<LValAST*>(ast.lval.get());
            int value = sym_table.lookup(lval->ident);
            auto num = new Integer(value);
            currentBlock->addValue(num);
            lastVal = num;
        }
        
    }
    void visit(NumberAST& ast) {
        auto num = new Integer(ast.value);
        currentBlock->addValue(num);
        lastVal = num;
    }
};
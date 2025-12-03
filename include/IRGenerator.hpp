#pragma once
#include "ast.hpp" 
#include "ir.hpp"  
#include <cassert> 

class IRGenerator {
public:
    std::unique_ptr<Program> program;

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
        if(ast.stmt)
        {
        auto retStmt = static_cast<ReturnStmtAST*>(ast.stmt.get());
        
        if(retStmt)
        {
        visit(*retStmt);
        }
    }
    } 
    // void visit(StmtAST& ast) {
    //     visit(*expNode);
    // }
    void visit(ReturnStmtAST& ast) {
        if(ast.exp){
            auto exp= static_cast<ExpAST*>(ast.exp.get());
            visit(*exp);
            auto retInst = new ReturnInst(lastVal);
            currentBlock->addInst(retInst);
        }
    }
    void visit(ExpAST& ast) {
        if(ast.unary_exp){
            auto unaryExp= static_cast<UnaryExpAST*>(ast.unary_exp.get());
            visit(*unaryExp);
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
                inst = new Binary(OpType::Sub, zero, operand, newTemp());
            } 
            else if (op == '!') {
                auto zero = new Integer(0);
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
    }
    void visit(NumberAST& ast) {
        lastVal = new Integer(ast.value);
    }
};
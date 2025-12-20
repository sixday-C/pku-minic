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
            auto decl = static_cast<DeclAST*>(block_item->decl.get());
            visit(*decl);
        }
        else if (block_item->stmt) {
            auto stmt = static_cast<StmtAST*>(block_item->stmt.get());
            visit(*stmt);
        }  
    }
    } 
    void visit(DeclAST& ast) {
        if (ast.const_decl) {
            auto const_decl = static_cast<ConstDeclAST*>(ast.const_decl.get());
            visit(*const_decl);
        }
        else if (ast.var_decl) {
            auto var_decl = static_cast<VarDeclAST*>(ast.var_decl.get());
            visit(*var_decl);
        }
    }
    void visit(ConstDeclAST& ast) {
        for (auto& const_def : ast.const_defs) {
            auto constDef = static_cast<ConstDefAST*>(const_def.get());
            visit(*constDef);
        }
    }
    void visit(ConstDefAST& ast) {
        auto const_init_val = static_cast<ConstInitValAST*>(ast.const_init_val.get());
        visit(*const_init_val);
        int value = lastVal->type == Type::Int32 ? static_cast<Integer*>(lastVal)->value : 0;
        sym_table.insertConst(ast.ident, value);
    }
     void visit(ConstInitValAST& ast) {
        int value = ast.const_exp->evalConst(sym_table);
        auto num = new Integer(value);
        currentBlock->addValue(num);
        lastVal = num;
    }
    void visit(VarDeclAST& ast) {
        for (auto& var_def : ast.var_defs) {
            auto varDef = static_cast<VarDefAST*>(var_def.get());
            visit(*varDef);
        }
    }
    void visit(VarDefAST& ast) {
        std::string uniqueName = sym_table.makeUniqueName(ast.ident);
        auto allocInst = new AllocInst(uniqueName);
        currentBlock->addInst(allocInst);
        sym_table.insertVar(ast.ident, allocInst);

        if (ast.init_val) {
            visit(*static_cast<InitValAST*>(ast.init_val.get()));
            auto storeInst = new StoreInst(lastVal, allocInst);
            currentBlock->addInst(storeInst);
        }
    }
    void visit(InitValAST& ast) {
        visit(*static_cast<ExpAST*>(ast.exp.get()));
    }
    void visit(StmtAST& ast) {
        switch(ast.type){
            case StmtAST::StmtType::Assign:
            {
            auto lval= static_cast<LValAST*>(ast.lval.get());
            Value* address = sym_table.lookupVar(lval->ident);
            visit(*static_cast<ExpAST*>(ast.exp.get()));
            Value* value = lastVal;
            auto storeInst = new StoreInst(value, address);
            currentBlock->addInst(storeInst);
                break;
            }
            case StmtAST::StmtType::Exp:
            {
                if(ast.exp){
                    auto exp= static_cast<ExpAST*>(ast.exp.get());
                    visit(*exp);
                }
                break;
            }
            case StmtAST::StmtType::Block:
                {
                    auto block = static_cast<BlockAST*>(ast.block.get());
                    sym_table.enterScope();
                    visit(*block);
                    sym_table.exitScope();
                    break;
                }
            case StmtAST::StmtType::Return:
                {
                    if(ast.exp){
                        auto exp= static_cast<ExpAST*>(ast.exp.get());
                        visit(*exp);
                    }
                    auto retInst = new ReturnInst(lastVal);
                    currentBlock->addInst(retInst);
                    break;
                }
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
            const auto& info = sym_table.lookup(lval->ident);
            if(info.type==SymbolInfo::CONST){
                int const_value = info.const_value;
                auto num = new Integer(const_value);
                currentBlock->addValue(num);
                lastVal = num;
                return;
            }
            else if(info.type==SymbolInfo::VAR){
                //load 指令生成
                auto loadInst = new LoadInst(info.var_alloc, newTemp());
                currentBlock->addInst(loadInst);
                lastVal = loadInst;
                return;
            }
        }
        
    }
    void visit(NumberAST& ast) {
        auto num = new Integer(ast.value);
        currentBlock->addValue(num);
        lastVal = num;
    }
};
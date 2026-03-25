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
    int blockCounter=0;//basicblock 命名计数器

    std::string newTemp() {
        return "%" + std::to_string(tempCounter++);
    }
    std::string newBlockLabel(const std::string& prefix) {
    return "%" + prefix + "_" + std::to_string(blockCounter++);
}
    bool blockHasTerminator(BasicBlock* block) {
        if (block->insts.empty()) {
            return false;
        }
        auto& lastInst = block->insts.back();
        return lastInst->op == OpType::Br || 
        lastInst->op == OpType::Jump ||
        lastInst->op == OpType::Ret;
    }
public:
    IRGenerator() : program(std::make_unique<Program>()) {}

    std::unique_ptr<Program> getProgram() {
        return std::move(program);
    }

void visit(CompUnitAST* ast){
    for (const auto& funcDef : ast->func_defs) {
        visit(*static_cast<FuncDefAST*>(funcDef.get()));
    }
}

void visit(FuncDefAST& ast) {
    tempCounter = 0;
    blockCounter = 0;

    //获取 AST 的函数，记录函数的类型
    auto funcTypeNode = static_cast<FuncTypeAST*>(ast.func_type.get());
    Type retType = (funcTypeNode->type == "void") ? Type::Void : Type::Int32;
    
    //创建 IR函数对象
    auto func = std::make_unique<Function>("@" + ast.ident, retType);
    currentFunc = func.get();
    sym_table.insertFunc(ast.ident, retType);

    //创建入口基本块
    auto entryBlock = new BasicBlock("%entry");
    currentBlock = entryBlock;
    func->addBlock(entryBlock);

    sym_table.enterScope();

    //给函数参数命名 生成alloc和store指令，并插入符号表
    for (auto &p : ast.func_fparams) {
        auto paramAST = static_cast<FuncFParamAST*>(p.get());
        std::string pName = "%" + paramAST->ident; 
        
        func->params.push_back({pName, Type::Int32});

        auto alloc = new AllocInst(sym_table.makeUniqueName(paramAST->ident));
        currentBlock->addInst(alloc);
        auto store = new StoreInst(new Parameter(pName), alloc);
        currentBlock->addInst(store);
        
        sym_table.insertVar(paramAST->ident, alloc);
    }

    auto blockNode = static_cast<BlockAST*>(ast.block.get());
    visit(*blockNode);

    if (!blockHasTerminator(currentBlock)) {
        if (retType == Type::Void) {
            currentBlock->addInst(new ReturnInst(nullptr)); 
        } else {
            currentBlock->addInst(new ReturnInst(new Integer(0)));
        }
    }

    sym_table.exitScope();
    program->funcs.push_back(std::move(func));
}
    
    void visit(BlockAST& ast) {
        for(auto& item : ast.block_items){
            if (blockHasTerminator(currentBlock)) {
                break; 
            }
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
                        auto retInst = new ReturnInst(lastVal);
                        currentBlock->addInst(retInst);
                    }
                    else {
                        auto retInst = new ReturnInst(nullptr);
                        currentBlock->addInst(retInst);
                    }
                    
                    
                    break;
                }
            case StmtAST::StmtType::IfThen://if (Exp) Stmt
                {   
                    //计算 Exp
                    visit (*static_cast<ExpAST*>(ast.exp.get()));
                    Value* condition=lastVal;

                    //创建 then 和 end 块
                    auto thenBlock=new BasicBlock(newBlockLabel("then"));
                    auto endBlock=new BasicBlock(newBlockLabel("end"));

                    //添加条件分支
                    auto brInst=new BranchInst(condition, thenBlock, endBlock);
                    currentBlock->addInst(brInst);

                    //处理 then 块
                    currentFunc->addBlock(thenBlock);
                    currentBlock=thenBlock;
                    visit(*static_cast<StmtAST*>(ast.then_stmt.get()));

                    //then 块结束后跳转到 end 块 (如果没有终止指令就生成 jump)
                    if (!blockHasTerminator(currentBlock)) {
                        auto jumpToEnd=new JumpInst(endBlock);
                        currentBlock->addInst(jumpToEnd);
                    }

                    //处理 end 块
                    currentFunc->addBlock(endBlock);
                    currentBlock=endBlock;

                    break;
                }
            case StmtAST::StmtType::IfElse://if(Exp)Stmt else Stmt
                {
                    //计算 Exp
                    visit (*static_cast<ExpAST*>(ast.exp.get()));
                    Value* condition=lastVal;

                    //创建 then、else 和 end 块
                    auto thenBlock=new BasicBlock(newBlockLabel("then"));
                    auto elseBlock=new BasicBlock(newBlockLabel("else"));
                    auto endBlock=new BasicBlock(newBlockLabel("end"));

                    //添加条件分支
                    auto brInst=new BranchInst(condition, thenBlock, elseBlock);
                    currentBlock->addInst(brInst);

                    //处理 then 块
                    currentFunc->addBlock(thenBlock);
                    currentBlock=thenBlock;
                    visit(*static_cast<StmtAST*>(ast.then_stmt.get()));

                    //then 块结束后跳转到 end 块  //如果没有终止指令就生成 jump
                    if (!blockHasTerminator(currentBlock)) {
                    auto jumpToEnd=new JumpInst(endBlock);
                    currentBlock->addInst(jumpToEnd);
                    }
                    //处理 else 块
                    currentFunc->addBlock(elseBlock);
                    currentBlock=elseBlock;
                    visit(*static_cast<StmtAST*>(ast.else_stmt.get()));

                    //else 块结束后跳转到 end 块  //如果没有终止指令就生成 jump
                    if (!blockHasTerminator(currentBlock)) {
                    auto jumpToEnd2=new JumpInst(endBlock);
                    currentBlock->addInst(jumpToEnd2);
                    }

                    //处理 end 块
                    currentFunc->addBlock(endBlock);
                    currentBlock=endBlock;
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
            // 基础情形：LAndExp -> EqExp
            // 没有左递归时，直接计算当前 EqExp，结果放到 lastVal。
            auto eqExp= static_cast<EqExpAST*>(ast.eq_exp.get());
            visit(*eqExp);
        }
        else if(ast.land_exp && ast.eq_exp){
            // a && b
            //int result=0;
            //if(a !=0 ){
            //   result=(b !=0);
            //   }  

            //@land_result = alloc i32
            auto resultAlloc = new AllocInst(sym_table.makeUniqueName("land_result"));   
            currentBlock->addInst(resultAlloc);
            
            //%0 = load @a
            visit(*static_cast<LAndExpAST*>(ast.land_exp.get()));
            Value* left = lastVal;

            //%1 = ne %0, 0
            auto zero = new Integer(0);
            currentBlock->addValue(zero);
            auto lhs_bool =new Binary(OpType::Ne, left, zero, newTemp());
            currentBlock->addInst(lhs_bool);

            //store 0, @land_result
            auto storeZero = new StoreInst(zero, resultAlloc);
            currentBlock->addInst(storeZero);

            //创建 rightBlock 和 endBlock
            auto rightBlock=new BasicBlock(newBlockLabel("land_right"));
            auto endBlock=new BasicBlock(newBlockLabel("land_end"));

            // 条件分支：left 真 -> rightBlock；left 假 -> endBlock（短路）。
            auto brInst=new BranchInst(lhs_bool, rightBlock, endBlock);
            currentBlock->addInst(brInst);

            //处理 rightBlock
            currentFunc->addBlock(rightBlock);
            currentBlock=rightBlock;
            visit(*static_cast<EqExpAST*>(ast.eq_exp.get()));
            Value* right = lastVal;

            //%2 = load @b
            auto rhs_bool = new Binary(OpType::Ne, right, zero, newTemp());
            currentBlock->addInst(rhs_bool);

            //%3 = ne %2, 0
            auto storeRhs=new StoreInst(rhs_bool, resultAlloc);
            currentBlock->addInst(storeRhs);
            
            // rightBlock 结束后跳转到汇合块。
            currentBlock->addInst(new JumpInst(endBlock));

            //生成并切换到汇合块。
            currentFunc->addBlock(endBlock);
            currentBlock = endBlock;

            // 读取最终结果，作为整个 && 表达式的值。
            auto finalRes = new LoadInst(resultAlloc, newTemp());
            currentBlock->addInst(finalRes);
            lastVal = finalRes;
            }
    }
    void visit(LOrExpAST& ast){
        if(ast.land_exp && !ast.lor_exp){
            auto landExp= static_cast<LAndExpAST*>(ast.land_exp.get());
            visit(*landExp);
        }
        else if(ast.lor_exp && ast.land_exp){
            // a || b
            //int result=1;
            //if(a ==0 ){
            //   result=(b !=0);
            //   }
            auto resultAlloc = new AllocInst(sym_table.makeUniqueName("lor_result"));   
            currentBlock->addInst(resultAlloc);

            visit(*static_cast<LOrExpAST*>(ast.lor_exp.get()));
            Value* left = lastVal;

            auto zero = new Integer(0);
            currentBlock->addValue(zero);
            auto lhs_bool =new Binary(OpType::Ne, left, zero, newTemp());
            currentBlock->addInst(lhs_bool);

            auto one = new Integer(1);
            currentBlock->addValue(one);
            currentBlock->addInst(new StoreInst(one, resultAlloc));

            auto nextBlock=new BasicBlock(newBlockLabel("lor_next"));
            auto endBlock=new BasicBlock(newBlockLabel("lor_end"));

            auto brInst=new BranchInst(lhs_bool, endBlock, nextBlock);
            currentBlock->addInst(brInst);


            currentFunc->addBlock(nextBlock);
            currentBlock=nextBlock;
            visit(*static_cast<LAndExpAST*>(ast.land_exp.get()));
            Value* right = lastVal;

            auto rhs_bool = new Binary(OpType::Ne, right, zero, newTemp());
            currentBlock->addInst(rhs_bool);
            auto storeRhs=new StoreInst(rhs_bool, resultAlloc);
            currentBlock->addInst(storeRhs);
            currentBlock->addInst(new JumpInst(endBlock));
            currentFunc->addBlock(endBlock);
            currentBlock = endBlock;

            auto finalRes = new LoadInst(resultAlloc, newTemp());
            currentBlock->addInst(finalRes);
            lastVal = finalRes;

            
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
        else if(ast.type == UnaryExpAST::UnaryType::Call){
            //函数调用
            std::vector<Value*> args;
            for(auto& arg : ast.func_args){
                visit(*static_cast<ExpAST*>(arg.get()));
                args.push_back(lastVal);
            }
            Type retType = sym_table.lookupFunc(ast.ident);
            std::string callName = (retType == Type::Void) ? "" : newTemp();
            auto callInst = new CallInst("@" + ast.ident, args, retType, callName);
            currentBlock->addInst(callInst);
            lastVal = callInst;
            return ;
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
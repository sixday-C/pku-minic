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
    struct LoopContext {
        BasicBlock* condBlock;
        BasicBlock* endBlock;
    };
    std::vector<LoopContext> loopStack;
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
    IRGenerator() : program(std::make_unique<Program>()) {
        setupLibraryFunctions();
    }
    void setupLibraryFunctions() {
        auto addLib = [&](const std::string& name, Type ret, std::vector<Type> params = {}) {
            sym_table.insertFunc(name, ret, params);
            program->decls.push_back({"@" + name, ret, params});
        };

        addLib("getint", Type::Int32);
        addLib("getch", Type::Int32);
        addLib("getarray", Type::Int32, {Type::Pointer});
        
        addLib("putint", Type::Void, {Type::Int32});
        addLib("putch", Type::Void, {Type::Int32});
        addLib("putarray", Type::Void, {Type::Int32, Type::Pointer});
        
        addLib("starttime", Type::Void);
        addLib("stoptime", Type::Void);
    }

    std::unique_ptr<Program> getProgram() {
        return std::move(program);
    }

void visit(CompUnitAST* ast){
    for (auto &item : ast->items) {
    if (auto func_ptr = dynamic_cast<FuncDefAST*>(item.get())) {
        visit(*func_ptr);
    } 
    else if (auto decl_ptr = dynamic_cast<DeclAST*>(item.get())) {
        visitGlobalDecl(decl_ptr);
    }
}
}

void visitGlobalDecl(DeclAST* ast) {
    //全局常量 const int a,b...;
    if (ast->const_decl) {
        auto const_decl = static_cast<ConstDeclAST*>(ast->const_decl.get());
        for (auto& const_def_base : const_decl->const_defs) {
            auto def = static_cast<ConstDefAST*>(const_def_base.get());
            
            if (def->array_size) { // const int a [12] = {...};
                int size = def->array_size->evalConst(sym_table);
                std::vector<int> values;
                //初始值先填满
                auto init_list = static_cast<ConstInitValAST*>(def->const_init_val.get());
                for (auto &item : init_list->init_list) {
                    values.push_back(item->evalConst(sym_table));
                }
                // 补零
                while (values.size() < (size_t)size) values.push_back(0);
                //生成 IR 里的全局分配对象
                auto gConst = std::make_unique<GlobalAlloc>("@" + def->ident, values, size);
                //存入符号表：记住名字、地址和长度
                sym_table.insertConstArray(def->ident, gConst.get(), size);
                //存入程序集：确保最后打印 IR 时会有这一行 global 定义
                program->globalValues.push_back(std::move(gConst));
            } 
            else { 
                // const int a=5,直接算出来存符号表即可，不需要生成 global 指令
                int val = def->const_init_val->evalConst(sym_table);
                sym_table.insertConst(def->ident, val);
            }
        }
    }
    // 全局变量 int a,b...; 或者 int a[12]
    else if (ast->var_decl) {
        auto var_decl = static_cast<VarDeclAST*>(ast->var_decl.get());
        for (auto& var_def_base : var_decl->var_defs) {
            auto def = static_cast<VarDefAST*>(var_def_base.get());
            
            if (def->array_size) { // int a [12] = {...};
                int size = def->array_size->evalConst(sym_table);
                std::vector<int> values;
                //初始值先填满
                if (def->init_val) {
                    auto init_ast = static_cast<InitValAST*>(def->init_val.get());
                    for (auto &item : init_ast->init_list) {
                        values.push_back(item->evalConst(sym_table));
                    }
                }
                // 补零
                while (values.size() < (size_t)size) values.push_back(0);
                //生成 IR 里的全局分配对象
                auto gVar = std::make_unique<GlobalAlloc>("@" + def->ident, values, size);
                //存入符号表：记住名字、地址和长度
                sym_table.insertVar(def->ident, gVar.get(), true, size);
                //存入程序集：确保最后打印 IR 时会有这一行 global 定义
                program->globalValues.push_back(std::move(gVar));
            } 
            else { // int a=5;
                int initVal = 0;
                if (def->init_val) {
                    initVal = static_cast<InitValAST*>(def->init_val.get())->exp->evalConst(sym_table);
                }
                auto gVar = std::make_unique<GlobalAlloc>("@" + def->ident, initVal);
                sym_table.insertVar(def->ident, gVar.get(), false);
                program->globalValues.push_back(std::move(gVar));
            }
        }
    }
}

void visit(FuncDefAST& ast) {
    tempCounter = 0;
    blockCounter = 0;

    //获取 AST 的函数，记录函数的类型
    auto funcTypeNode = static_cast<BTypeAST*>(ast.func_type.get());
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
        int value = const_init_val->evalConst(sym_table);
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

    //局部变量定义
    void visit(VarDefAST& ast) {
    std::string uniqueName = sym_table.makeUniqueName(ast.ident);
    
    if (ast.array_size) { // int a[12] = {...};
        int size = ast.array_size->evalConst(sym_table);
        auto allocInst = new AllocInst(uniqueName, size); 
        currentBlock->addInst(allocInst);
        sym_table.insertVar(ast.ident, allocInst, true); 

        if (ast.init_val) {//int arr[3] = {1,2,3}; 有初始值列表 也可能是 int arr[3] = {1,a+3}; 需要 visit
            auto init_ast = static_cast<InitValAST*>(ast.init_val.get());
            
        /*  %0 = getelemptr @arr_0, 0  // 找 0 号房
            store 1, %0                // 存数字 1
            %1 = getelemptr @arr_0, 1  // 找 1 号房
            store 2, %1                // 存数字 2
            %2 = getelemptr @arr_0, 2  // 找 2 号房
            store 3, %2                // 存数字 3
         */

            for (size_t i = 0; i < init_ast->init_list.size(); ++i) {
                // 访问初始值表达式（可能是a+3，所以要 visit）
                auto sub_init = static_cast<InitValAST*>(init_ast->init_list[i].get());
                visit(*static_cast<ExpAST*>(sub_init->exp.get())); 
                Value* val = lastVal;//拿到结果
                // 寻址并存入
                auto gep = new GetElemPtrInst(allocInst, new Integer(i), newTemp());
                currentBlock->addInst(gep);
                currentBlock->addInst(new StoreInst(val, gep));
            }
            // 2. 补零：处理剩下的空间
            for (size_t i = init_ast->init_list.size(); i < (size_t)size; ++i) {
                auto gep = new GetElemPtrInst(allocInst, new Integer(i), newTemp());
                currentBlock->addInst(gep);
                currentBlock->addInst(new StoreInst(new Integer(0), gep));
            }
        }
    } 

    else { // int a=5; 或者 int a; 后者没有初始值
        /*
        %0 = alloc i32               // 生成分配指令，分配一个 i32 大小的空间，名字叫 %0
        store 5, %0                  // 生成存储指令，把数字 5 存到 %0 里
        */
        auto allocInst = new AllocInst(uniqueName);
        currentBlock->addInst(allocInst);
        sym_table.insertVar(ast.ident, allocInst, false);
        if (ast.init_val) {
            visit(*static_cast<InitValAST*>(ast.init_val.get()));
            currentBlock->addInst(new StoreInst(lastVal, allocInst));
        }
    }
    }

    void visit(InitValAST& ast) {
        visit(*static_cast<ExpAST*>(ast.exp.get()));
    }
    void visit(StmtAST& ast) {
        switch(ast.type){
            case StmtAST::StmtType::Assign: {
            // 1. 获取左值 (LVal) 和 右值 (Exp) 的 AST 节点
            auto lval = static_cast<LValAST*>(ast.lval.get());
            auto exp = static_cast<ExpAST*>(ast.exp.get());

            // 2. 在符号表中查找这个变量的信息
            const auto& info = sym_table.lookup(lval->ident);

            // --- [语义检查] 防止张冠李戴 ---
            // 检查 A: 常量不能被赋值 (如: const int a = 1; a = 2; ERROR!)
            if (info.kind == SymbolInfo::CONST) {
                std::cerr << "Error: Cannot assign to constant '" << lval->ident << "'." << std::endl;
                exit(1);
            }
            // 检查 B: 数组名不能被整体赋值 (如: int arr[3]; arr = 1; ERROR!)
            if (info.is_array && !lval->index) {
                std::cerr << "Error: Cannot assign to array name '" << lval->ident << "' directly." << std::endl;
                exit(1);
            }
            // 检查 C: 非数组不能带下标 (如: int a; a[1] = 1; ERROR!)
            if (!info.is_array && lval->index) {
                std::cerr << "Error: Scalar '" << lval->ident << "' cannot be indexed." << std::endl;
                exit(1);
            }
            // --- [核心逻辑] 确定存放的目的地 (Target Address) ---
            // 拿到变量的基地址 (例如 @a 或 @arr)
            Value* targetAddr = info.var_alloc;
            // 如果用户带了下标，比如 a[i] = ...
            if (lval->index) {
                // A. 计算下标表达式 i 的值。结果存入 lastVal
                visit(*static_cast<ExpAST*>(lval->index.get()));
                Value* idxVal = lastVal;
                // B. 生成 GEP 指令：定位到具体的“抽屉”
                // IR 示例: %0 = getelemptr @arr, %idxVal
                auto gep = new GetElemPtrInst(targetAddr, idxVal, newTemp());
                currentBlock->addInst(gep);
                // C. [关键] 将目的地更新为这个精准的房间地址
                targetAddr = gep;
            }
            // --- [核心逻辑] 计算并存入数据 ---
            // 3. 计算等号右边表达式 (Exp) 的值。结果存入 lastVal
            visit(*exp); 
            Value* valToStore = lastVal;
            // 4. 生成 Store 指令：将值搬进目的地
            // IR 示例: store %valToStore, %targetAddr
            // 如果是 a = 1, targetAddr 就是 @a
            // 如果是 a[i] = 1, targetAddr 就是刚才算的 %0
            currentBlock->addInst(new StoreInst(valToStore, targetAddr));
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
            case StmtAST::StmtType::While:
            {
            auto condBlock = new BasicBlock(newBlockLabel("while_cond"));
            auto bodyBlock = new BasicBlock(newBlockLabel("while_body"));
            auto endBlock = new BasicBlock(newBlockLabel("while_end"));
            currentBlock->addInst(new JumpInst(condBlock));
            currentFunc->addBlock(condBlock);
            currentBlock = condBlock;
            visit(*static_cast<ExpAST*>(ast.exp.get()));
            Value* condition = lastVal;

            currentBlock->addInst(new BranchInst(condition, bodyBlock, endBlock));
            loopStack.push_back({condBlock, endBlock});

            currentFunc->addBlock(bodyBlock);
            currentBlock = bodyBlock;
            visit(*static_cast<StmtAST*>(ast.while_stmt.get()));

            if (!blockHasTerminator(currentBlock)) {
            currentBlock->addInst(new JumpInst(condBlock));
            }
            loopStack.pop_back();
            currentFunc->addBlock(endBlock);
            currentBlock = endBlock; 
            break;
            }
           case StmtAST::StmtType::Break: {
            if (loopStack.empty()) {
                std::cerr << "语义错误：'break' 语句不在循环内！" << std::endl;
                exit(1); 
            }
            BasicBlock* target = loopStack.back().endBlock;
            currentBlock->addInst(new JumpInst(target));
            break;
        }

        case StmtAST::StmtType::Continue: {
            if (loopStack.empty()) {
                std::cerr << "语义错误：'continue' 语句不在循环内！" << std::endl;
                exit(1);
            }

            BasicBlock* target = loopStack.back().condBlock;
            currentBlock->addInst(new JumpInst(target));
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
    // --- 1. (Number) ---
    // 例如: return 5;
    if (ast.number) {
        auto num = static_cast<NumberAST*>(ast.number.get());
        visit(*num); // 这会把 lastVal 设为 new Integer(val)
        return;
    }
    // --- 2. 处理嵌套表达式 ( Exp ) ---
    // 例如: return (a + 1);
    else if (ast.exp) {
        auto exp = static_cast<ExpAST*>(ast.exp.get());
        visit(*exp); // 递归处理括号内部，结果会更新在 lastVal
        return;
    }
    // --- 3. 处理左值访问 (LVal) ---
    // 这是最复杂的部分，涉及标量、数组、常量和变量的区分
    else if (ast.lval) {
        auto lval = static_cast<LValAST*>(ast.lval.get());
        const auto& info = sym_table.lookup(lval->ident);
        // --- 语义检查 (Semantic Check) ---
        // 检查 A: 数组名不能直接参与运算 (如: int b = arr + 1;)
        if (info.is_array && !lval->index) {
            std::cerr << "Error: Array '" << lval->ident << "' must be indexed." << std::endl;
            exit(1);
        }
        // 检查 B: 标量不能带下标 (如: int a; int b = a;)
        if (!info.is_array && lval->index) {
            std::cerr << "Error: Scalar '" << lval->ident << "' cannot be indexed." << std::endl;
            exit(1);
        }
        // --- 分支 A: 标量常量 (Constant Substitution) ---
        // 如果是 const int x = 5; 且不是数组
        // 我们直接“变魔术”，把变量名换成具体的数字，不产生 IR 指令
        if (info.kind == SymbolInfo::CONST && !info.is_array) {
            lastVal = new Integer(info.const_value);
            return;
        }
        // --- 分支 B: 变量 或 常量数组 (Memory Access) ---
        // 无论是 int a 还是 const int arr，只要是数组或者普通变量，
        // 都需要去内存里“取货”，所以流程是一样的。
        // 1. 获取基地址 (由之前的 alloc 生成的地址，例如 @a 或 @arr)
        Value* addr = info.var_alloc;

        // 2. [关键] 数组寻址 (GetElementPtr)
        // 如果用户写了下标，比如 arr[i]
        if (lval->index) {
            // A. 计算下标表达式的值。结果存入 lastVal (可能是常数 2，也可能是变量 %5)
            visit(*static_cast<ExpAST*>(lval->index.get()));
            Value* idxVal = lastVal;
            // B. 生成 GEP 指令：算出精准的房间门牌号
            // IR 示例: %0 = getelemptr @arr, %idxVal
            auto gep = new GetElemPtrInst(addr, idxVal, newTemp());
            currentBlock->addInst(gep);
            // C. 更新目标地址：现在我们要去刚才算出的“房间”取货，而不是在“大门口”
            addr = gep;
        }
        // 3. 生成 Load 指令：从地址 addr 中读取数值
        // IR 示例: %1 = load %0 (或者是 load @a)
        auto loadInst = new LoadInst(addr, newTemp());
        currentBlock->addInst(loadInst);
        // 4. 将 Load 后的结果（临时变量 %1）交给 lastVal
        // 这样后续的加减乘除就能直接用这个 %1 了
        lastVal = loadInst;
    }
    }
    void visit(NumberAST& ast) {
        auto num = new Integer(ast.value);
        currentBlock->addValue(num);
        lastVal = num;
    }
};
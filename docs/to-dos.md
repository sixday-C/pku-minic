# 做实验的心态:
1.mark不懂的问题.没有log=没有发生.

2.不要卡在一个地方,先进行最小实现.



# 文件结构:
## 头文件include
    ast.hpp                 //ast数据结构
    ir.hpp                  //ir数据结构
    ir_gen.hpp        //ast->ir
    rv_gen.hpp     //ir->rv

## 源文件src
    ast.cpp                 //ast   成员函数实现
    ir_gen.cpp        //ast->ir
    riscv_gen.cpp     //ir->rv


## 编译main
    main.cpp


# To do:

- [x] 改IR结构 10.1
- [x] 查看wdlin,YOOkoishi的pku-minic 10.1
- [x] 重构AST：分离hpp和cpp实现 10.4
- [x] 重构IR：分离hpp和cpp实现 10.4
- [x] 优化UnaryOp：从AST节点改为char类型 10.4

# To-Do: 当前优先级任务

## 1. 完善IR框架
- [x] 修复Binary类定义 (缺少构造函数、dump函数、访问修饰符)
- [x] 在IR.cpp中添加Binary类的实现
- [ ] 统一临时变量命名策略 (%t0, %t1, %t2...)

## 2. AST到IR转换重构
- [ ] 重新设计AST的IR()方法，生成IR对象而非文本输出
- [ ] 创建IRBuilder类管理IR构建过程
- [ ] 重构全局变量g_ir和g_last_val的管理方式
- [ ] 实现AST节点到IR Value的映射

## 3. 编译流程集成
- [ ] 修改main.cpp整合新的IR生成流程
- [ ] 确保AST解析 → IR生成 → 代码生成的完整流程
- [ ] 添加命令行参数支持不同编译阶段的输出

## 4. 代码质量改进
- [ ] 添加错误处理机制 (语法错误、语义错误)
- [ ] 统一代码风格和注释
- [ ] 添加调试输出选项

## 5. 测试验证
- [ ] 编写单元测试验证AST构建
- [ ] 测试IR生成的正确性
- [ ] 端到端测试：源码 → AST → IR → 汇编
- [ ] 验证一元表达式的各种组合：
  - return 42; → ret 42  
  - return -6; → ret -6
  - return !0; → ret 1
  - return +(- -!6); → 正确的IR指令序列

## 6. 后续扩展准备
- [ ] 为二元表达式扩展做准备
- [ ] 考虑变量声明和使用的设计
- [ ] 函数调用和参数传递的框架设计

# 已完成的AST重构
- [x] ExpAST：持有 UnaryExp  
- [x] PrimaryExpAST：要么是 Number，要么是 (Exp)
- [x] UnaryExpAST：支持PrimaryExp 或 (UnaryOp + UnaryExp)
- [x] UnaryOp改为char类型，简化实现
- [x] 所有AST类的实现分离到ast.cpp
- [x] 语法分析器sysy.y适配新的UnaryOp设计
    

    
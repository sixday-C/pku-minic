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

# 已完成的AST重构
- [x] ExpAST：持有 UnaryExp  
- [x] PrimaryExpAST：要么是 Number，要么是 (Exp)
- [x] UnaryExpAST：支持PrimaryExp 或 (UnaryOp + UnaryExp)
- [x] UnaryOp改为char类型，简化实现
- [x] 所有AST类的实现分离到ast.cpp
- [x] 语法分析器sysy.y适配新的UnaryOp设计


# 目前进度
Program (IR)
  └─ Function
      └─ BasicBlock
          └─ Instructions (Binary, Return, ...)

                ↓ 转换

RVProg (RISC-V)
  └─ RVFunc
      └─ RVBlock
          └─ RVInst (RVLi, RVSub, RVMv, ...)
    

    
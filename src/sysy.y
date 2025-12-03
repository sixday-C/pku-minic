%code requires {
  #include <memory>
  #include <string>
  #include "../include/ast.hpp"   
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "../include/ast.hpp"   
// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }


// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
   BaseAST *ast_val;//ast声明
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN
%token <str_val> IDENT
%token <int_val> INT_CONST


// 非终结符的类型定义
%type <ast_val> CompUnit FuncDef FuncType Block Stmt Number
%type <ast_val> Exp PrimaryExp UnaryExp 

%%

CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
  ;

FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  } 
  ;

// 同上, 不再解释
FuncType
  : INT { $$ = new FuncTypeAST("int"); }
  ;

Block
  : '{' Stmt '}' {
    auto b=new BlockAST();
    b->stmt=std::unique_ptr<BaseAST>($2);
    $$=b;
  }
  ;



Stmt
  : RETURN Exp ';' {
    auto s=new ReturnStmtAST();
    s->exp=std::unique_ptr<BaseAST>($2);
    $$=s;
  }
;

Exp
  : UnaryExp {
    auto e=new ExpAST();
    e->unary_exp=std::unique_ptr<BaseAST>($1);
    $$=e;
  }
  ;
  
PrimaryExp
  : '(' Exp ')' {
    auto p=new PrimaryExpAST();
    p->exp=std::unique_ptr<BaseAST>($2);
    $$ = p;
  }
  | Number {
    auto p=new PrimaryExpAST();
    p->number=std::unique_ptr<BaseAST>($1);
    $$ = p;
  }
UnaryExp
  : PrimaryExp {
    auto u=new UnaryExpAST();
    u->primary_exp=std::unique_ptr<BaseAST>($1);
    u->unary_op = '\0';  // 没有一元操作符时设为空字符
    $$=u;
  }
  | '+' UnaryExp {
    auto u=new UnaryExpAST();
    u->unary_op = '+';
    u->unary_exp=std::unique_ptr<BaseAST>($2);
    $$=u;
  }
  | '-' UnaryExp {
    auto u=new UnaryExpAST();
    u->unary_op = '-';
    u->unary_exp=std::unique_ptr<BaseAST>($2);
    $$=u; 
  }
  | '!' UnaryExp {
    auto u=new UnaryExpAST();
    u->unary_op = '!';
    u->unary_exp=std::unique_ptr<BaseAST>($2);
    $$=u; 
  }
;
Number
  : INT_CONST {
    $$ = new NumberAST($1);
  }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}

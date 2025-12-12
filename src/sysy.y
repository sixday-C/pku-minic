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
%token INT RETURN LE GE EQ NE LOR LAND CONST 
%token <str_val> IDENT
%token <int_val> INT_CONST


// 非终结符的类型定义
%type <ast_val> CompUnit FuncDef FuncType Block Stmt Number
%type <ast_val> Exp PrimaryExp UnaryExp 
%type <ast_val> MulExp AddExp
%type <ast_val> LOrExp RelExp EqExp LAndExp 
%type <ast_val> LVal
%type <ast_val> BlockItemList BlockItem Decl ConstDecl BType ConstDef ConstInitVal ConstExp ConstDefList
%type <ast_val> VarDecl VarDef InitVal VarDefList

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
  : '{' '}' {
    auto b=new BlockAST();
    $$=b;
  }
  |'{' BlockItemList '}' {
    $$=$2;
  }
  ;

BlockItemList
  : BlockItem {
    auto b=new BlockAST();
    b->block_items.push_back(std::unique_ptr<BaseAST>($1));
    $$=b;
  }
  | BlockItemList BlockItem {
    auto b=static_cast<BlockAST*>($1);
    b->block_items.push_back(std::unique_ptr<BaseAST>($2));
    $$=$1;
  }
  ;

BlockItem  
   : Decl {
    auto b=new BlockItemAST();
    b->decl=std::unique_ptr<BaseAST>($1);
    $$=b;
   }
   | Stmt{
    auto b=new BlockItemAST();
    b->stmt=std::unique_ptr<BaseAST>($1);
    $$=b;
   };

Decl 
  : ConstDecl{
    auto d=new DeclAST();
    d->const_decl=std::unique_ptr<BaseAST>($1);
    $$=d;
  }
  | VarDecl{
    auto d=new DeclAST();
    d->var_decl=std::unique_ptr<BaseAST>($1);
    $$=d;
  }
  ;

VarDecl
  : BType VarDefList ';'{
    auto d=static_cast<VarDeclAST*>($2);
    d->b_type=std::unique_ptr<BaseAST>($1);
    $$=d;
  }
  ;
  
VarDefList
  : VarDef {
    auto v=new VarDeclAST();
    v->var_defs.push_back(std::unique_ptr<BaseAST>($1));
    $$=v;
  }
  | VarDefList ',' VarDef {
    auto v=static_cast<VarDeclAST*>($1);
    v->var_defs.push_back(std::unique_ptr<BaseAST>($3));
    $$=$1;
  }
  ;

VarDef
  : IDENT {
    auto v=new VarDefAST();
    v->ident=*unique_ptr<string>($1);
    v->init_val=nullptr;
    $$=v;
  }
  | IDENT '=' InitVal {
    auto v=new VarDefAST();
    v->ident=*unique_ptr<string>($1);
    v->init_val=std::unique_ptr<BaseAST>($3);
    $$=v;
  }
  ;

InitVal
  : Exp{
    auto i=new InitValAST();
    i->exp=std::unique_ptr<BaseAST>($1);
    $$=i;
  }
  ;

ConstDecl
  : CONST BType ConstDefList ';'{
    auto d=static_cast<ConstDeclAST*>($3);
    d->b_type=std::unique_ptr<BaseAST>($2);
    $$=d;
  }
  ;
  ConstDefList
  : ConstDef {
    auto c=new ConstDeclAST();
    c->const_defs.push_back(std::unique_ptr<BaseAST>($1));
    $$=c;
  }
  | ConstDefList ',' ConstDef {
    auto c=static_cast<ConstDeclAST*>($1);
    c->const_defs.push_back(std::unique_ptr<BaseAST>($3));
    $$=$1;
  }
  ;

BType
  : INT {
    $$ = new BTypeAST("int");
  }
  ;

ConstDef
  : IDENT '=' ConstInitVal {
    auto c=new ConstDefAST();
    c->ident=*unique_ptr<string>($1);
    c->const_init_val=std::unique_ptr<BaseAST>($3);
    $$=c;
  }
  ;

ConstInitVal
  : ConstExp{
    auto c=new ConstInitValAST();
    c->const_exp=std::unique_ptr<BaseAST>($1);
    $$=c;
  }
  ;

ConstExp
 :Exp{
    auto c=new ConstExpAST();
    c->exp=std::unique_ptr<BaseAST>($1);
    $$=c;
  }
  ;

Stmt
  :LVal '=' Exp ';' {
    auto s=new StmtAST();
    s->lval=std::unique_ptr<BaseAST>($1);
    s->exp=std::unique_ptr<BaseAST>($3);
    $$=s;
  }
  | RETURN Exp ';' {
    auto s=new StmtAST();
    s->exp=std::unique_ptr<BaseAST>($2);
    $$=s;
  }
;

Exp
  : LOrExp{
    auto e=new ExpAST();
    e->lor_exp=std::unique_ptr<BaseAST>($1);
    $$=e;
  }
;

LOrExp
  : LAndExp {
    auto l=new LOrExpAST();
    l->land_exp=std::unique_ptr<BaseAST>($1);
    $$=l;
  }
  | LOrExp LOR LAndExp {
    auto l=new LOrExpAST();
    l->lor_exp=std::unique_ptr<BaseAST>($1);
    l->land_exp=std::unique_ptr<BaseAST>($3);
    $$=l;
  }
  ;

LAndExp
  : EqExp {
    auto l=new LAndExpAST();
    l->eq_exp=std::unique_ptr<BaseAST>($1);
    $$=l;
  }
  | LAndExp LAND EqExp {
    auto l=new LAndExpAST();
    l->land_exp=std::unique_ptr<BaseAST>($1);
    l->eq_exp=std::unique_ptr<BaseAST>($3);
    $$=l;
  }
  ;

EqExp
  : RelExp {
    auto e=new EqExpAST();
    e->rel_exp=std::unique_ptr<BaseAST>($1);
    $$=e;
  }
  | EqExp EQ RelExp {
    auto e=new EqExpAST();
    e->eq_exp=std::unique_ptr<BaseAST>($1);
    e->rel_exp=std::unique_ptr<BaseAST>($3);
    e->eq_op="==";
    $$=e;
  }
  | EqExp NE RelExp {
    auto e=new EqExpAST();
    e->eq_exp=std::unique_ptr<BaseAST>($1);
    e->rel_exp=std::unique_ptr<BaseAST>($3);
    e->eq_op="!=";
    $$=e;
  }
  ;

  RelExp
  : AddExp {
    auto r=new RelExpAST();
    r->add_exp=std::unique_ptr<BaseAST>($1);
    $$=r;
  }
  | RelExp '<' AddExp {
    auto r=new RelExpAST();
    r->rel_exp=std::unique_ptr<BaseAST>($1);
    r->add_exp=std::unique_ptr<BaseAST>($3);
    r->rel_op="<";
    $$=r;
  }
  | RelExp '>' AddExp {
    auto r=new RelExpAST();
    r->rel_exp=std::unique_ptr<BaseAST>($1);
    r->add_exp=std::unique_ptr<BaseAST>($3);
    r->rel_op=">";
    $$=r;
  }
  | RelExp LE AddExp {
    auto r=new RelExpAST();
    r->rel_exp=std::unique_ptr<BaseAST>($1);
    r->add_exp=std::unique_ptr<BaseAST>($3);  
    r->rel_op="<=";
    $$=r;
  }
  | RelExp GE AddExp {
    auto r=new RelExpAST();
    r->rel_exp=std::unique_ptr<BaseAST>($1);
    r->add_exp=std::unique_ptr<BaseAST>($3);  
    r->rel_op=">=";
    $$=r;
  }

AddExp
  : MulExp {
    auto a=new AddExpAST();
    a->mul_exp=std::unique_ptr<BaseAST>($1);
    $$=a;
  }
  | AddExp '+' MulExp {
    auto a=new AddExpAST();
    a->add_exp=std::unique_ptr<BaseAST>($1);
    a->mul_exp=std::unique_ptr<BaseAST>($3);
    a->add_op='+';
    $$=a;
  }
  | AddExp '-' MulExp {
    auto a=new AddExpAST();
    a->add_exp=std::unique_ptr<BaseAST>($1);
    a->mul_exp=std::unique_ptr<BaseAST>($3);
    a->add_op='-';
    $$=a;
  }

MulExp
  : UnaryExp {
    auto m=new MulExpAST();
    m->unary_exp=std::unique_ptr<BaseAST>($1);
    $$=m;
  }
  | MulExp '*' UnaryExp {
    auto m=new MulExpAST();
    m->mul_exp=std::unique_ptr<BaseAST>($1);
    m->unary_exp=std::unique_ptr<BaseAST>($3);
    m->mul_op='*';
    $$=m;
  }
  | MulExp '/' UnaryExp {
    auto m=new MulExpAST();
    m->mul_exp=std::unique_ptr<BaseAST>($1);
    m->unary_exp=std::unique_ptr<BaseAST>($3);
    m->mul_op='/';
    $$=m;
  }
  | MulExp '%' UnaryExp {
    auto m=new MulExpAST();
    m->mul_exp=std::unique_ptr<BaseAST>($1);
    m->unary_exp=std::unique_ptr<BaseAST>($3);
    m->mul_op='%';
    $$=m;
  }
  
PrimaryExp
  : '(' Exp ')' {
    auto p=new PrimaryExpAST();
    p->exp=std::unique_ptr<BaseAST>($2);
    $$ = p;
  }
  | LVal {
    auto p=new PrimaryExpAST();
    p->lval=std::unique_ptr<BaseAST>($1);
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
LVal
  : IDENT {
    auto l=new LValAST();
    l->ident=*std::unique_ptr<std::string>($1);
    $$=l;
  }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}

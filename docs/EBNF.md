# EBNF

CompUnit  ::= FuncDef;
FuncDef   ::= FuncType IDENT "(" ")" Block;
FuncType  ::= "int";
Block     ::= "{" Stmt "}"%;
Number    ::= INT_CONST;
Stmt        ::= "return" Exp ";";
Exp         ::= UnaryExp;
PrimaryExp  ::= "(" Exp ")" | Number; //6   (---6)
Number      ::= INT_CONST;
UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp; // ----6
UnaryOp     ::= "+" | "-" | "!";
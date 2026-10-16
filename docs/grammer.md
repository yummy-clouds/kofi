# Kofi Formal Grammar

```ebnf
Program         ::= Function* EOF ;

Function        ::= "fc" IDENT "(" [ ParamList ] ")" [ "=>" Type ] "{" Statement* "}" ;

ParamList       ::= Param ( "," Param )* ;
Param           ::= IDENT ":" Type ;

Type            ::= "int" | "flt" | "bln" | "stg" | "nil" ;

Statement       ::= VarDeclStmt
                  | AssignStmt
                  | ForStmt
                  | IfStmt
                  | ReturnStmt
                  | BreakStmt
                  | ContinueStmt
                  | ExprStmt ;

VarDeclStmt     ::= "var" IDENT [ ":" Type ] [ "=" Expression ] [ ";" ] ;

AssignStmt      ::= IDENT "=" Expression [ ";" ] ;

ForStmt         ::= "for" [ ForHeader ] "{" Statement* "}" ;

ForHeader       ::= "(" ForClauses ")"
                  | ForClauses
                  | "(" Expression ")"
                  | Expression ;

ForClauses      ::= [ ForInit ] ";" [ Expression ] ";" [ ForPost ] ;
ForInit         ::= VarDeclStmt | AssignStmt | Expression ;
ForPost         ::= AssignStmt | Expression ;

IfStmt          ::= "if" [ "(" ] Expression [ ")" ] "{" Statement* "}" [ "else" ( IfStmt | "{" Statement* "}" ) ] ;

ReturnStmt      ::= "return" [ Expression ] [ ";" ] ;

BreakStmt       ::= "break" [ ";" ] ;

ContinueStmt    ::= "continue" [ ";" ] ;

ExprStmt        ::= Expression [ ";" ] ;

Expression      ::= LogicalOr ;

LogicalOr       ::= LogicalAnd ( "||" LogicalAnd )* ;

LogicalAnd      ::= Equality ( "&&" Equality )* ;

Equality        ::= Comparison ( ( "==" | "!=" ) Comparison )* ;

Comparison      ::= Term ( ( "<" | "<=" | ">" | ">=" ) Term )* ;

Term            ::= Factor ( ( "+" | "-" ) Factor )* ;

Factor          ::= Unary ( ( "*" | "/" | "%" ) Unary )* ;

Unary           ::= ( "-" | "!" ) Unary
                  | Primary ;

Primary         ::= INT_LIT
                  | FLT_LIT
                  | STR_LIT
                  | "true"
                  | "false"
                  | "nil"
                  | IDENT [ "(" [ ArgList ] ")" | "=" Expression ]
                  | "(" Expression ")" ;

ArgList         ::= Expression ( "," Expression )* ;
```

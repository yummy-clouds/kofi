#ifndef AST_H
#define AST_H

typedef struct Expr Expr;
typedef struct Stmt Stmt;
typedef struct Function Function;
typedef struct Program Program;

typedef enum {
    EXPR_INT_LIT,
    EXPR_FLT_LIT,
    EXPR_BLN_LIT,
    EXPR_STR_LIT,
    EXPR_NIL_LIT,
    EXPR_IDENT,
    EXPR_CALL,
    EXPR_BINARY,
    EXPR_UNARY,
    EXPR_ASSIGN
} ExprType;

struct Expr {
    ExprType type;
    union {
        long long int_val;
        double flt_val;
        int bool_val;
        char* str_val;
        char* ident_val;
        struct {
            char* callee;
            Expr** args;
            int arg_count;
        } call;
        struct {
            char op[4];
            Expr* left;
            Expr* right;
        } binary;
        struct {
            char op[4];
            Expr* operand;
        } unary;
        struct {
            char* name;
            Expr* value;
        } assign;
    };
};

typedef enum {
    STMT_RETURN,
    STMT_EXPR,
    STMT_VAR_DECL,
    STMT_ASSIGN,
    STMT_FOR,
    STMT_IF,
    STMT_BREAK,
    STMT_CONTINUE
} StmtType;

struct Stmt {
    StmtType type;
    union {
        Expr* expr; // STMT_RETURN, STMT_EXPR
        struct {
            char* name;
            char* type; // "int", "flt", "bln", "stg", "nil", or NULL (inferred)
            Expr* init; // NULL if uninitialized
        } var_decl;
        struct {
            char* name;
            Expr* value;
        } assign;
        struct {
            Stmt* init;     // optional init statement (or NULL)
            Expr* cond;     // optional condition (NULL => infinite loop)
            Stmt* post;     // optional post statement (or NULL)
            Stmt* body;     // loop body statements
            int body_count;
        } for_loop;
        struct {
            Expr* cond;
            Stmt* then_stmts;
            int then_count;
            Stmt* else_stmts;
            int else_count;
        } if_stmt;
    };
};

typedef struct {
    char* name;
    char* type;
} Param;

struct Function {
    char* name;
    Param* params;
    int param_count;
    char* ret_type; // e.g., "int", "flt", "bln", "stg", "nil", or NULL (void)
    Stmt* stmts;
    int stmt_count;
};

struct Program {
    Function* functions;
    int func_count;
};

#endif
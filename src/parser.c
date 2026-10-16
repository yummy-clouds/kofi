// src/parser.c
#include "parser.h"
#include "lexer.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    Token* tokens;
    int capacity;
    int count;
    int current;
} TokenStream;

static TokenStream g_stream;

static void advance() {
    if (g_stream.current < g_stream.count) {
        g_stream.current++;
    }
}

static Token peek() {
    return g_stream.tokens[g_stream.current];
}

static Token consume(TokenType type, const char* message) {
    Token t = peek();
    if (t.type == type) {
        advance();
        return t;
    }
    error(t.line, message);
    return t;
}

static char* token_to_string(Token t) {
    char* s = malloc(t.length + 1);
    memcpy(s, t.start, t.length);
    s[t.length] = '\0';
    return s;
}

static int is_type_token(TokenType type) {
    return type == TOKEN_INT_KW ||
           type == TOKEN_FLT_KW ||
           type == TOKEN_BLN_KW ||
           type == TOKEN_STG_KW ||
           type == TOKEN_NIL_KW;
}

static Expr* parse_expression();
static Stmt parse_statement();

static Expr* parse_primary() {
    Token t = peek();
    if (t.type == TOKEN_INT_LIT) {
        advance();
        char* s = token_to_string(t);
        Expr* e = malloc(sizeof(Expr));
        e->type = EXPR_INT_LIT;
        e->int_val = atoll(s);
        free(s);
        return e;
    }
    if (t.type == TOKEN_FLT_LIT) {
        advance();
        char* s = token_to_string(t);
        Expr* e = malloc(sizeof(Expr));
        e->type = EXPR_FLT_LIT;
        e->flt_val = atof(s);
        free(s);
        return e;
    }
    if (t.type == TOKEN_TRUE) {
        advance();
        Expr* e = malloc(sizeof(Expr));
        e->type = EXPR_BLN_LIT;
        e->bool_val = 1;
        return e;
    }
    if (t.type == TOKEN_FALSE) {
        advance();
        Expr* e = malloc(sizeof(Expr));
        e->type = EXPR_BLN_LIT;
        e->bool_val = 0;
        return e;
    }
    if (t.type == TOKEN_NIL_KW) {
        advance();
        Expr* e = malloc(sizeof(Expr));
        e->type = EXPR_NIL_LIT;
        return e;
    }
    if (t.type == TOKEN_STR_LIT) {
        advance();
        // Strip quotes
        int len = t.length >= 2 ? t.length - 2 : 0;
        char* s = malloc(len + 1);
        if (len > 0) {
            memcpy(s, t.start + 1, len);
        }
        s[len] = '\0';
        Expr* e = malloc(sizeof(Expr));
        e->type = EXPR_STR_LIT;
        e->str_val = s;
        return e;
    }
    if (t.type == TOKEN_IDENT) {
        advance();
        char* name = token_to_string(t);
        // Check if function call
        if (peek().type == TOKEN_LPAREN) {
            advance(); // consume '('
            Expr** args = NULL;
            int arg_count = 0;
            if (peek().type != TOKEN_RPAREN) {
                do {
                    args = realloc(args, sizeof(Expr*) * (arg_count + 1));
                    args[arg_count++] = parse_expression();
                } while (peek().type == TOKEN_COMMA && (advance(), 1));
            }
            consume(TOKEN_RPAREN, "Expected ')' after arguments.");
            Expr* e = malloc(sizeof(Expr));
            e->type = EXPR_CALL;
            e->call.callee = name;
            e->call.args = args;
            e->call.arg_count = arg_count;
            return e;
        }
        // Check if assignment expression: ident = expr
        if (peek().type == TOKEN_ASSIGN) {
            advance(); // consume '='
            Expr* val = parse_expression();
            Expr* e = malloc(sizeof(Expr));
            e->type = EXPR_ASSIGN;
            e->assign.name = name;
            e->assign.value = val;
            return e;
        }
        Expr* e = malloc(sizeof(Expr));
        e->type = EXPR_IDENT;
        e->ident_val = name;
        return e;
    }
    if (t.type == TOKEN_LPAREN) {
        advance();
        Expr* e = parse_expression();
        consume(TOKEN_RPAREN, "Expected ')' after expression.");
        return e;
    }

    error(t.line, "Expected expression.");
    return NULL;
}

static Expr* parse_unary() {
    Token t = peek();
    if (t.type == TOKEN_MINUS || t.type == TOKEN_BANG) {
        advance();
        Expr* operand = parse_unary();
        Expr* e = malloc(sizeof(Expr));
        e->type = EXPR_UNARY;
        char* op = token_to_string(t);
        strncpy(e->unary.op, op, sizeof(e->unary.op) - 1);
        e->unary.op[sizeof(e->unary.op) - 1] = '\0';
        free(op);
        e->unary.operand = operand;
        return e;
    }
    return parse_primary();
}

static Expr* parse_factor() {
    Expr* left = parse_unary();
    while (peek().type == TOKEN_STAR || peek().type == TOKEN_SLASH || peek().type == TOKEN_PERCENT) {
        Token op_tok = peek();
        advance();
        Expr* right = parse_unary();
        Expr* e = malloc(sizeof(Expr));
        e->type = EXPR_BINARY;
        char* op = token_to_string(op_tok);
        strncpy(e->binary.op, op, sizeof(e->binary.op) - 1);
        e->binary.op[sizeof(e->binary.op) - 1] = '\0';
        free(op);
        e->binary.left = left;
        e->binary.right = right;
        left = e;
    }
    return left;
}

static Expr* parse_term() {
    Expr* left = parse_factor();
    while (peek().type == TOKEN_PLUS || peek().type == TOKEN_MINUS) {
        Token op_tok = peek();
        advance();
        Expr* right = parse_factor();
        Expr* e = malloc(sizeof(Expr));
        e->type = EXPR_BINARY;
        char* op = token_to_string(op_tok);
        strncpy(e->binary.op, op, sizeof(e->binary.op) - 1);
        e->binary.op[sizeof(e->binary.op) - 1] = '\0';
        free(op);
        e->binary.left = left;
        e->binary.right = right;
        left = e;
    }
    return left;
}

static Expr* parse_comparison() {
    Expr* left = parse_term();
    while (peek().type == TOKEN_LT || peek().type == TOKEN_LTE ||
           peek().type == TOKEN_GT || peek().type == TOKEN_GTE) {
        Token op_tok = peek();
        advance();
        Expr* right = parse_term();
        Expr* e = malloc(sizeof(Expr));
        e->type = EXPR_BINARY;
        char* op = token_to_string(op_tok);
        strncpy(e->binary.op, op, sizeof(e->binary.op) - 1);
        e->binary.op[sizeof(e->binary.op) - 1] = '\0';
        free(op);
        e->binary.left = left;
        e->binary.right = right;
        left = e;
    }
    return left;
}

static Expr* parse_equality() {
    Expr* left = parse_comparison();
    while (peek().type == TOKEN_EQ || peek().type == TOKEN_NEQ) {
        Token op_tok = peek();
        advance();
        Expr* right = parse_comparison();
        Expr* e = malloc(sizeof(Expr));
        e->type = EXPR_BINARY;
        char* op = token_to_string(op_tok);
        strncpy(e->binary.op, op, sizeof(e->binary.op) - 1);
        e->binary.op[sizeof(e->binary.op) - 1] = '\0';
        free(op);
        e->binary.left = left;
        e->binary.right = right;
        left = e;
    }
    return left;
}

static Expr* parse_and() {
    Expr* left = parse_equality();
    while (peek().type == TOKEN_AND) {
        advance();
        Expr* right = parse_equality();
        Expr* e = malloc(sizeof(Expr));
        e->type = EXPR_BINARY;
        strncpy(e->binary.op, "&&", sizeof(e->binary.op) - 1);
        e->binary.op[sizeof(e->binary.op) - 1] = '\0';
        e->binary.left = left;
        e->binary.right = right;
        left = e;
    }
    return left;
}

static Expr* parse_or() {
    Expr* left = parse_and();
    while (peek().type == TOKEN_OR) {
        advance();
        Expr* right = parse_and();
        Expr* e = malloc(sizeof(Expr));
        e->type = EXPR_BINARY;
        strncpy(e->binary.op, "||", sizeof(e->binary.op) - 1);
        e->binary.op[sizeof(e->binary.op) - 1] = '\0';
        e->binary.left = left;
        e->binary.right = right;
        left = e;
    }
    return left;
}

static Expr* parse_expression() {
    return parse_or();
}

static Stmt parse_var_decl_stmt() {
    consume(TOKEN_VAR, "Expected 'var'.");
    Token name_tok = consume(TOKEN_IDENT, "Expected variable name after 'var'.");
    char* name = token_to_string(name_tok);
    char* type = NULL;
    Expr* init = NULL;

    if (peek().type == TOKEN_COLON) {
        advance(); // consume ':'
        if (!is_type_token(peek().type)) {
            error(peek().line, "Expected type name (int, flt, bln, stg, nil) after ':'.");
        }
        Token type_tok = peek();
        advance();
        type = token_to_string(type_tok);
    }

    if (peek().type == TOKEN_ASSIGN) {
        advance(); // consume '='
        init = parse_expression();
    }

    if (peek().type == TOKEN_SEMICOLON) {
        advance();
    }

    Stmt s;
    s.type = STMT_VAR_DECL;
    s.var_decl.name = name;
    s.var_decl.type = type;
    s.var_decl.init = init;
    return s;
}

static Stmt parse_assign_stmt() {
    Token name_tok = consume(TOKEN_IDENT, "Expected variable name.");
    consume(TOKEN_ASSIGN, "Expected '=' in assignment.");
    Expr* val = parse_expression();
    if (peek().type == TOKEN_SEMICOLON) {
        advance();
    }
    Stmt s;
    s.type = STMT_ASSIGN;
    s.assign.name = token_to_string(name_tok);
    s.assign.value = val;
    return s;
}

static Stmt parse_for_stmt() {
    consume(TOKEN_FOR, "Expected 'for'.");

    Stmt* init = NULL;
    Expr* cond = NULL;
    Stmt* post = NULL;

    // Check if infinite loop: for { ... }
    if (peek().type == TOKEN_LBRACE) {
        // init, cond, post are all NULL
    } else {
        // Scan ahead to see if there is a semicolon before the opening '{'
        int has_semi = 0;
        for (int i = g_stream.current; i < g_stream.count; i++) {
            TokenType t = g_stream.tokens[i].type;
            if (t == TOKEN_LBRACE) break;
            if (t == TOKEN_SEMICOLON) {
                has_semi = 1;
                break;
            }
        }

        if (has_semi) {
            // Three-clause for loop: for [init]; [cond]; [post] { ... }
            int has_paren = 0;
            if (peek().type == TOKEN_LPAREN) {
                has_paren = 1;
                advance();
            }

            // 1. Init
            if (peek().type != TOKEN_SEMICOLON) {
                init = malloc(sizeof(Stmt));
                if (peek().type == TOKEN_VAR) {
                    *init = parse_var_decl_stmt();
                } else if (peek().type == TOKEN_IDENT &&
                           g_stream.current + 1 < g_stream.count &&
                           g_stream.tokens[g_stream.current + 1].type == TOKEN_ASSIGN) {
                    *init = parse_assign_stmt();
                } else {
                    Expr* e = parse_expression();
                    *init = (Stmt){STMT_EXPR, {.expr = e}};
                    if (peek().type == TOKEN_SEMICOLON) advance();
                }
            } else {
                advance(); // consume ';'
            }

            // 2. Condition
            if (peek().type != TOKEN_SEMICOLON) {
                cond = parse_expression();
            }
            consume(TOKEN_SEMICOLON, "Expected ';' after for-loop condition.");

            // 3. Post
            if (peek().type != TOKEN_LBRACE && (!has_paren || peek().type != TOKEN_RPAREN)) {
                post = malloc(sizeof(Stmt));
                if (peek().type == TOKEN_IDENT &&
                    g_stream.current + 1 < g_stream.count &&
                    g_stream.tokens[g_stream.current + 1].type == TOKEN_ASSIGN) {
                    *post = parse_assign_stmt();
                } else {
                    Expr* e = parse_expression();
                    *post = (Stmt){STMT_EXPR, {.expr = e}};
                    if (peek().type == TOKEN_SEMICOLON) advance();
                }
            }

            if (has_paren) {
                consume(TOKEN_RPAREN, "Expected ')' after for-loop header.");
            }
        } else {
            // Single-condition loop: for cond { ... }
            int has_paren = 0;
            if (peek().type == TOKEN_LPAREN) {
                has_paren = 1;
                advance();
            }
            cond = parse_expression();
            if (has_paren) {
                consume(TOKEN_RPAREN, "Expected ')' after condition.");
            }
        }
    }

    consume(TOKEN_LBRACE, "Expected '{' before loop body.");
    Stmt* body = NULL;
    int body_count = 0;
    while (peek().type != TOKEN_RBRACE && peek().type != TOKEN_EOF) {
        body = realloc(body, sizeof(Stmt) * (body_count + 1));
        body[body_count++] = parse_statement();
    }
    consume(TOKEN_RBRACE, "Expected '}' after loop body.");

    Stmt s;
    s.type = STMT_FOR;
    s.for_loop.init = init;
    s.for_loop.cond = cond;
    s.for_loop.post = post;
    s.for_loop.body = body;
    s.for_loop.body_count = body_count;
    return s;
}

static Stmt parse_if_stmt() {
    consume(TOKEN_IF, "Expected 'if'.");
    int has_paren = 0;
    if (peek().type == TOKEN_LPAREN) {
        has_paren = 1;
        advance();
    }
    Expr* cond = parse_expression();
    if (has_paren) {
        consume(TOKEN_RPAREN, "Expected ')' after if condition.");
    }

    consume(TOKEN_LBRACE, "Expected '{' before if body.");
    Stmt* then_stmts = NULL;
    int then_count = 0;
    while (peek().type != TOKEN_RBRACE && peek().type != TOKEN_EOF) {
        then_stmts = realloc(then_stmts, sizeof(Stmt) * (then_count + 1));
        then_stmts[then_count++] = parse_statement();
    }
    consume(TOKEN_RBRACE, "Expected '}' after if body.");

    Stmt* else_stmts = NULL;
    int else_count = 0;
    if (peek().type == TOKEN_ELSE) {
        advance();
        if (peek().type == TOKEN_IF) {
            else_stmts = malloc(sizeof(Stmt));
            else_stmts[0] = parse_if_stmt();
            else_count = 1;
        } else {
            consume(TOKEN_LBRACE, "Expected '{' before else body.");
            while (peek().type != TOKEN_RBRACE && peek().type != TOKEN_EOF) {
                else_stmts = realloc(else_stmts, sizeof(Stmt) * (else_count + 1));
                else_stmts[else_count++] = parse_statement();
            }
            consume(TOKEN_RBRACE, "Expected '}' after else body.");
        }
    }

    Stmt s;
    s.type = STMT_IF;
    s.if_stmt.cond = cond;
    s.if_stmt.then_stmts = then_stmts;
    s.if_stmt.then_count = then_count;
    s.if_stmt.else_stmts = else_stmts;
    s.if_stmt.else_count = else_count;
    return s;
}

static Stmt parse_statement() {
    Token t = peek();

    if (t.type == TOKEN_VAR) {
        return parse_var_decl_stmt();
    }
    if (t.type == TOKEN_FOR) {
        return parse_for_stmt();
    }
    if (t.type == TOKEN_IF) {
        return parse_if_stmt();
    }
    if (t.type == TOKEN_BREAK) {
        advance();
        if (peek().type == TOKEN_SEMICOLON) advance();
        return (Stmt){STMT_BREAK, {0}};
    }
    if (t.type == TOKEN_CONTINUE) {
        advance();
        if (peek().type == TOKEN_SEMICOLON) advance();
        return (Stmt){STMT_CONTINUE, {0}};
    }
    if (t.type == TOKEN_RETURN) {
        advance();
        Expr* expr = NULL;
        if (peek().type != TOKEN_SEMICOLON && peek().type != TOKEN_RBRACE) {
            expr = parse_expression();
        }
        if (peek().type == TOKEN_SEMICOLON) advance();
        return (Stmt){STMT_RETURN, {.expr = expr}};
    }

    // Check if assignment statement: ident = expr
    if (t.type == TOKEN_IDENT &&
        g_stream.current + 1 < g_stream.count &&
        g_stream.tokens[g_stream.current + 1].type == TOKEN_ASSIGN) {
        return parse_assign_stmt();
    }

    // Expression statement
    Expr* expr = parse_expression();
    if (peek().type == TOKEN_SEMICOLON) advance();
    return (Stmt){STMT_EXPR, {.expr = expr}};
}

static Function parse_function() {
    consume(TOKEN_FC, "Expected 'fc' keyword.");
    Token name_tok = consume(TOKEN_IDENT, "Expected function name.");
    char* name = token_to_string(name_tok);

    consume(TOKEN_LPAREN, "Expected '(' after function name.");
    Param* params = NULL;
    int param_count = 0;
    if (peek().type != TOKEN_RPAREN) {
        do {
            Token p_name = consume(TOKEN_IDENT, "Expected parameter name.");
            consume(TOKEN_COLON, "Expected ':' after parameter name.");
            if (!is_type_token(peek().type)) {
                error(peek().line, "Expected type name (int, flt, bln, stg, nil).");
            }
            Token p_type = peek();
            advance();
            params = realloc(params, sizeof(Param) * (param_count + 1));
            params[param_count++] = (Param){token_to_string(p_name), token_to_string(p_type)};
        } while (peek().type == TOKEN_COMMA && (advance(), 1));
    }
    consume(TOKEN_RPAREN, "Expected ')' after parameters.");

    char* ret_type = NULL;
    if (peek().type == TOKEN_ARROW) {
        advance();
        if (!is_type_token(peek().type)) {
            error(peek().line, "Expected return type (int, flt, bln, stg, nil) after '=>'.");
        }
        Token rt = peek();
        advance();
        ret_type = token_to_string(rt);
    }

    consume(TOKEN_LBRACE, "Expected '{' before function body.");
    Stmt* stmts = NULL;
    int stmt_count = 0;
    while (peek().type != TOKEN_RBRACE && peek().type != TOKEN_EOF) {
        stmts = realloc(stmts, sizeof(Stmt) * (stmt_count + 1));
        stmts[stmt_count++] = parse_statement();
    }
    consume(TOKEN_RBRACE, "Expected '}' after function body.");

    return (Function){name, params, param_count, ret_type, stmts, stmt_count};
}

Program* parse(const char* source) {
    Lexer lexer;
    lexer_init(&lexer, source);
    g_stream.tokens = malloc(sizeof(Token) * 64);
    g_stream.capacity = 64;
    g_stream.count = 0;
    g_stream.current = 0;

    Token token;
    do {
        token = lexer_next_token(&lexer);
        if (g_stream.count >= g_stream.capacity) {
            g_stream.capacity *= 2;
            g_stream.tokens = realloc(g_stream.tokens, sizeof(Token) * g_stream.capacity);
        }
        g_stream.tokens[g_stream.count++] = token;
    } while (token.type != TOKEN_EOF);

    Program* prog = malloc(sizeof(Program));
    prog->functions = NULL;
    prog->func_count = 0;

    while (peek().type != TOKEN_EOF) {
        prog->functions = realloc(prog->functions, sizeof(Function) * (prog->func_count + 1));
        prog->functions[prog->func_count++] = parse_function();
    }

    return prog;
}
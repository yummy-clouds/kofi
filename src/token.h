#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    TOKEN_EOF = 0,
    TOKEN_FC,          // fc
    TOKEN_RETURN,      // return
    TOKEN_VAR,         // var
    TOKEN_FOR,         // for
    TOKEN_IF,          // if
    TOKEN_ELSE,        // else
    TOKEN_BREAK,       // break
    TOKEN_CONTINUE,    // continue

    // Data types (5 types: int, flt, bln, stg, nil)
    TOKEN_INT_KW,      // int
    TOKEN_FLT_KW,      // flt
    TOKEN_BLN_KW,      // bln
    TOKEN_STG_KW,      // stg
    TOKEN_NIL_KW,      // nil

    // Literals & Identifiers
    TOKEN_IDENT,       // identifier
    TOKEN_INT_LIT,     // integer literal (e.g. 42)
    TOKEN_FLT_LIT,     // float literal (e.g. 3.14)
    TOKEN_STR_LIT,     // string literal (e.g. "hello")
    TOKEN_TRUE,        // true
    TOKEN_FALSE,       // false

    // Delimiters
    TOKEN_LPAREN,      // (
    TOKEN_RPAREN,      // )
    TOKEN_LBRACE,      // {
    TOKEN_RBRACE,      // }
    TOKEN_COLON,       // :
    TOKEN_SEMICOLON,   // ;
    TOKEN_COMMA,       // ,
    TOKEN_ARROW,       // =>

    // Operators
    TOKEN_ASSIGN,      // =
    TOKEN_EQ,          // ==
    TOKEN_NEQ,         // !=
    TOKEN_LT,          // <
    TOKEN_LTE,         // <=
    TOKEN_GT,          // >
    TOKEN_GTE,         // >=
    TOKEN_PLUS,        // +
    TOKEN_MINUS,       // -
    TOKEN_STAR,        // *
    TOKEN_SLASH,       // /
    TOKEN_PERCENT,     // %
    TOKEN_BANG,        // !
    TOKEN_AND,         // &&
    TOKEN_OR           // ||
} TokenType;

typedef struct {
    TokenType type;
    const char *start;
    int length;
    int line;
} Token;

#endif
// src/lexer.c
#include "lexer.h"
#include <string.h>

void lexer_init(Lexer* lexer, const char* source) {
    lexer->start = source;
    lexer->current = source;
    lexer->line = 1;
}

static int is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static int is_digit(char c) {
    return c >= '0' && c <= '9';
}

Token lexer_next_token(Lexer* lexer) {
    while (*lexer->current != '\0') {
        char c = *lexer->current;

        // Skip whitespace
        if (c == ' ' || c == '\r' || c == '\t') {
            lexer->current++;
            continue;
        }
        if (c == '\n') {
            lexer->line++;
            lexer->current++;
            continue;
        }

        // Single-line comment
        if (c == '/' && *(lexer->current + 1) == '/') {
            lexer->current += 2;
            while (*lexer->current != '\n' && *lexer->current != '\0') {
                lexer->current++;
            }
            continue;
        }

        // Multi-line comment
        if (c == '/' && *(lexer->current + 1) == '*') {
            lexer->current += 2;
            while (*lexer->current != '\0') {
                if (*lexer->current == '\n') lexer->line++;
                if (*lexer->current == '*' && *(lexer->current + 1) == '/') {
                    lexer->current += 2;
                    break;
                }
                lexer->current++;
            }
            continue;
        }

        lexer->start = lexer->current;

        // Identifiers and keywords
        if (is_alpha(c)) {
            while (is_alpha(*lexer->current) || is_digit(*lexer->current)) {
                lexer->current++;
            }
            int len = (int)(lexer->current - lexer->start);
            TokenType type = TOKEN_IDENT;

            if (len == 2 && memcmp(lexer->start, "fc", 2) == 0) type = TOKEN_FC;
            else if (len == 3 && memcmp(lexer->start, "var", 3) == 0) type = TOKEN_VAR;
            else if (len == 3 && memcmp(lexer->start, "for", 3) == 0) type = TOKEN_FOR;
            else if (len == 2 && memcmp(lexer->start, "if", 2) == 0) type = TOKEN_IF;
            else if (len == 4 && memcmp(lexer->start, "else", 4) == 0) type = TOKEN_ELSE;
            else if (len == 5 && memcmp(lexer->start, "break", 5) == 0) type = TOKEN_BREAK;
            else if (len == 8 && memcmp(lexer->start, "continue", 8) == 0) type = TOKEN_CONTINUE;
            else if (len == 6 && memcmp(lexer->start, "return", 6) == 0) type = TOKEN_RETURN;
            // The five data types
            else if (len == 3 && memcmp(lexer->start, "int", 3) == 0) type = TOKEN_INT_KW;
            else if (len == 3 && memcmp(lexer->start, "flt", 3) == 0) type = TOKEN_FLT_KW;
            else if (len == 3 && memcmp(lexer->start, "bln", 3) == 0) type = TOKEN_BLN_KW;
            else if (len == 3 && memcmp(lexer->start, "stg", 3) == 0) type = TOKEN_STG_KW;
            else if (len == 3 && memcmp(lexer->start, "nil", 3) == 0) type = TOKEN_NIL_KW;
            // Boolean literals
            else if (len == 4 && memcmp(lexer->start, "true", 4) == 0) type = TOKEN_TRUE;
            else if (len == 5 && memcmp(lexer->start, "false", 5) == 0) type = TOKEN_FALSE;

            return (Token){type, lexer->start, len, lexer->line};
        }

        // Numeric literals (int and flt)
        if (is_digit(c)) {
            while (is_digit(*lexer->current)) lexer->current++;
            if (*lexer->current == '.' && is_digit(*(lexer->current + 1))) {
                lexer->current++; // consume '.'
                while (is_digit(*lexer->current)) lexer->current++;
                return (Token){TOKEN_FLT_LIT, lexer->start, (int)(lexer->current - lexer->start), lexer->line};
            }
            return (Token){TOKEN_INT_LIT, lexer->start, (int)(lexer->current - lexer->start), lexer->line};
        }

        // String literals
        if (c == '"') {
            lexer->current++;
            while (*lexer->current != '"' && *lexer->current != '\0') {
                if (*lexer->current == '\n') lexer->line++;
                if (*lexer->current == '\\' && *(lexer->current + 1) != '\0') {
                    lexer->current++;
                }
                lexer->current++;
            }
            if (*lexer->current == '"') lexer->current++;
            return (Token){TOKEN_STR_LIT, lexer->start, (int)(lexer->current - lexer->start), lexer->line};
        }

        // Delimiters and operators
        lexer->current++;
        switch (c) {
            case '(': return (Token){TOKEN_LPAREN, lexer->start, 1, lexer->line};
            case ')': return (Token){TOKEN_RPAREN, lexer->start, 1, lexer->line};
            case '{': return (Token){TOKEN_LBRACE, lexer->start, 1, lexer->line};
            case '}': return (Token){TOKEN_RBRACE, lexer->start, 1, lexer->line};
            case ':': return (Token){TOKEN_COLON, lexer->start, 1, lexer->line};
            case ';': return (Token){TOKEN_SEMICOLON, lexer->start, 1, lexer->line};
            case ',': return (Token){TOKEN_COMMA, lexer->start, 1, lexer->line};
            case '+': return (Token){TOKEN_PLUS, lexer->start, 1, lexer->line};
            case '-': return (Token){TOKEN_MINUS, lexer->start, 1, lexer->line};
            case '*': return (Token){TOKEN_STAR, lexer->start, 1, lexer->line};
            case '/': return (Token){TOKEN_SLASH, lexer->start, 1, lexer->line};
            case '%': return (Token){TOKEN_PERCENT, lexer->start, 1, lexer->line};
            case '!':
                if (*lexer->current == '=') {
                    lexer->current++;
                    return (Token){TOKEN_NEQ, lexer->start, 2, lexer->line};
                }
                return (Token){TOKEN_BANG, lexer->start, 1, lexer->line};
            case '=':
                if (*lexer->current == '>') {
                    lexer->current++;
                    return (Token){TOKEN_ARROW, lexer->start, 2, lexer->line};
                }
                if (*lexer->current == '=') {
                    lexer->current++;
                    return (Token){TOKEN_EQ, lexer->start, 2, lexer->line};
                }
                return (Token){TOKEN_ASSIGN, lexer->start, 1, lexer->line};
            case '<':
                if (*lexer->current == '=') {
                    lexer->current++;
                    return (Token){TOKEN_LTE, lexer->start, 2, lexer->line};
                }
                return (Token){TOKEN_LT, lexer->start, 1, lexer->line};
            case '>':
                if (*lexer->current == '=') {
                    lexer->current++;
                    return (Token){TOKEN_GTE, lexer->start, 2, lexer->line};
                }
                return (Token){TOKEN_GT, lexer->start, 1, lexer->line};
            case '&':
                if (*lexer->current == '&') {
                    lexer->current++;
                    return (Token){TOKEN_AND, lexer->start, 2, lexer->line};
                }
                break;
            case '|':
                if (*lexer->current == '|') {
                    lexer->current++;
                    return (Token){TOKEN_OR, lexer->start, 2, lexer->line};
                }
                break;
        }

        return (Token){TOKEN_EOF, lexer->start, 1, lexer->line};
    }

    return (Token){TOKEN_EOF, lexer->current, 0, lexer->line};
}
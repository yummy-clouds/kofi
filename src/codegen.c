// src/codegen.c
#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* map_type(const char* kofi_type) {
    if (!kofi_type) return "void";
    if (strcmp(kofi_type, "int") == 0) return "int";
    if (strcmp(kofi_type, "flt") == 0) return "double";
    if (strcmp(kofi_type, "bln") == 0) return "bool";
    if (strcmp(kofi_type, "stg") == 0) return "const char*";
    if (strcmp(kofi_type, "nil") == 0) return "void*";
    return kofi_type;
}

static const char* default_val(const char* kofi_type) {
    if (!kofi_type) return "NULL";
    if (strcmp(kofi_type, "int") == 0) return "0";
    if (strcmp(kofi_type, "flt") == 0) return "0.0";
    if (strcmp(kofi_type, "bln") == 0) return "false";
    if (strcmp(kofi_type, "stg") == 0) return "NULL";
    if (strcmp(kofi_type, "nil") == 0) return "NULL";
    return "0";
}

static void emit_indent(FILE* f, int indent) {
    for (int i = 0; i < indent; i++) {
        fprintf(f, "    ");
    }
}

static void emit_expr(FILE* f, Expr* e) {
    if (!e) return;
    switch (e->type) {
        case EXPR_INT_LIT:
            fprintf(f, "%lld", e->int_val);
            break;
        case EXPR_FLT_LIT:
            fprintf(f, "%f", e->flt_val);
            break;
        case EXPR_BLN_LIT:
            fprintf(f, "%s", e->bool_val ? "true" : "false");
            break;
        case EXPR_STR_LIT:
            fprintf(f, "\"%s\"", e->str_val);
            break;
        case EXPR_NIL_LIT:
            fprintf(f, "nil");
            break;
        case EXPR_IDENT:
            fprintf(f, "%s", e->ident_val);
            break;
        case EXPR_ASSIGN:
            fprintf(f, "(%s = ", e->assign.name);
            emit_expr(f, e->assign.value);
            fprintf(f, ")");
            break;
        case EXPR_CALL:
            if (strcmp(e->call.callee, "printf") == 0 ||
                strcmp(e->call.callee, "prinf") == 0 ||
                strcmp(e->call.callee, "print") == 0) {
                if (e->call.arg_count == 0) {
                    fprintf(f, "printf(\"\")");
                } else {
                    for (int i = 0; i < e->call.arg_count; i++) {
                        fprintf(f, "_kofi_printf_single(");
                        emit_expr(f, e->call.args[i]);
                        fprintf(f, ")");
                        if (i < e->call.arg_count - 1) {
                            fprintf(f, ", ");
                        }
                    }
                }
            } else if (strcmp(e->call.callee, "scanf") == 0) {
                if (e->call.arg_count == 0) {
                    fprintf(f, "0");
                } else if (e->call.arg_count == 1) {
                    // Single variable without prompt: scanf(a)
                    fprintf(f, "_kofi_scanf(\"\", &(");
                    emit_expr(f, e->call.args[0]);
                    fprintf(f, "))");
                } else if (e->call.args[0]->type == EXPR_STR_LIT) {
                    // Prompt provided: scanf("enter a num", a)
                    for (int i = 1; i < e->call.arg_count; i++) {
                        fprintf(f, "_kofi_scanf(");
                        if (i == 1) {
                            emit_expr(f, e->call.args[0]);
                        } else {
                            fprintf(f, "\"\"");
                        }
                        fprintf(f, ", &(");
                        emit_expr(f, e->call.args[i]);
                        fprintf(f, "))");
                        if (i < e->call.arg_count - 1) {
                            fprintf(f, ", ");
                        }
                    }
                } else {
                    // Multiple variables without prompt: scanf(a, b)
                    for (int i = 0; i < e->call.arg_count; i++) {
                        fprintf(f, "_kofi_scanf(\"\", &(");
                        emit_expr(f, e->call.args[i]);
                        fprintf(f, "))");
                        if (i < e->call.arg_count - 1) {
                            fprintf(f, ", ");
                        }
                    }
                }
            } else {
                fprintf(f, "%s(", e->call.callee);
                for (int i = 0; i < e->call.arg_count; i++) {
                    emit_expr(f, e->call.args[i]);
                    if (i < e->call.arg_count - 1) fprintf(f, ", ");
                }
                fprintf(f, ")");
            }
            break;
        case EXPR_BINARY:
            fprintf(f, "(");
            emit_expr(f, e->binary.left);
            fprintf(f, " %s ", e->binary.op);
            emit_expr(f, e->binary.right);
            fprintf(f, ")");
            break;
        case EXPR_UNARY:
            fprintf(f, "(%s", e->unary.op);
            emit_expr(f, e->unary.operand);
            fprintf(f, ")");
            break;
    }
}

static void emit_stmt_inline(FILE* f, Stmt* s) {
    if (!s) return;
    switch (s->type) {
        case STMT_VAR_DECL: {
            const char* c_type = s->var_decl.type ? map_type(s->var_decl.type) : "__auto_type";
            if (s->var_decl.init) {
                fprintf(f, "%s %s = ", c_type, s->var_decl.name);
                emit_expr(f, s->var_decl.init);
            } else {
                fprintf(f, "%s %s = %s", c_type, s->var_decl.name, default_val(s->var_decl.type));
            }
            break;
        }
        case STMT_ASSIGN:
            fprintf(f, "%s = ", s->assign.name);
            emit_expr(f, s->assign.value);
            break;
        case STMT_EXPR:
            emit_expr(f, s->expr);
            break;
        default:
            break;
    }
}

static void emit_stmt(FILE* f, Stmt* s, int indent) {
    if (!s) return;
    switch (s->type) {
        case STMT_RETURN:
            emit_indent(f, indent);
            fprintf(f, "return");
            if (s->expr) {
                fprintf(f, " ");
                emit_expr(f, s->expr);
            }
            fprintf(f, ";\n");
            break;
        case STMT_VAR_DECL:
            emit_indent(f, indent);
            emit_stmt_inline(f, s);
            fprintf(f, ";\n");
            break;
        case STMT_ASSIGN:
            emit_indent(f, indent);
            emit_stmt_inline(f, s);
            fprintf(f, ";\n");
            break;
        case STMT_EXPR:
            emit_indent(f, indent);
            emit_expr(f, s->expr);
            fprintf(f, ";\n");
            break;
        case STMT_BREAK:
            emit_indent(f, indent);
            fprintf(f, "break;\n");
            break;
        case STMT_CONTINUE:
            emit_indent(f, indent);
            fprintf(f, "continue;\n");
            break;
        case STMT_IF:
            emit_indent(f, indent);
            fprintf(f, "if (");
            emit_expr(f, s->if_stmt.cond);
            fprintf(f, ") {\n");
            for (int j = 0; j < s->if_stmt.then_count; j++) {
                emit_stmt(f, &s->if_stmt.then_stmts[j], indent + 1);
            }
            emit_indent(f, indent);
            fprintf(f, "}");
            if (s->if_stmt.else_count > 0) {
                fprintf(f, " else {\n");
                for (int j = 0; j < s->if_stmt.else_count; j++) {
                    emit_stmt(f, &s->if_stmt.else_stmts[j], indent + 1);
                }
                emit_indent(f, indent);
                fprintf(f, "}\n");
            } else {
                fprintf(f, "\n");
            }
            break;
        case STMT_FOR:
            emit_indent(f, indent);
            if (s->for_loop.init || s->for_loop.post) {
                // Three-clause for loop: for (init; cond; post)
                fprintf(f, "for (");
                if (s->for_loop.init) {
                    emit_stmt_inline(f, s->for_loop.init);
                }
                fprintf(f, "; ");
                if (s->for_loop.cond) {
                    emit_expr(f, s->for_loop.cond);
                }
                fprintf(f, "; ");
                if (s->for_loop.post) {
                    emit_stmt_inline(f, s->for_loop.post);
                }
                fprintf(f, ") {\n");
            } else if (s->for_loop.cond) {
                // Condition-only while-style loop
                fprintf(f, "while (");
                emit_expr(f, s->for_loop.cond);
                fprintf(f, ") {\n");
            } else {
                // Infinite loop
                fprintf(f, "while (1) {\n");
            }
            for (int j = 0; j < s->for_loop.body_count; j++) {
                emit_stmt(f, &s->for_loop.body[j], indent + 1);
            }
            emit_indent(f, indent);
            fprintf(f, "}\n");
            break;
    }
}

void generate_code(Program* prog, const char* out_c_path) {
    FILE* f = fopen(out_c_path, "w");
    if (!f) {
        perror("Could not create C output file");
        exit(1);
    }

    fprintf(f, "#include <stdio.h>\n");
    fprintf(f, "#include <stdbool.h>\n");
    fprintf(f, "#include <stdlib.h>\n");
    fprintf(f, "#include <string.h>\n\n");

    fprintf(f, "#define nil ((void*)0)\n\n");

    // Output: printf support (without automatic newline)
    fprintf(f, "static inline void _kofi_printf_bln(bool b) { printf(\"%%s\", b ? \"true\" : \"false\"); }\n");
    fprintf(f, "static inline void _kofi_printf_int(long long v) { printf(\"%%lld\", v); }\n");
    fprintf(f, "static inline void _kofi_printf_flt(double v) { printf(\"%%g\", v); }\n");
    fprintf(f, "static inline void _kofi_printf_stg(const char* s) { printf(\"%%s\", s ? s : \"nil\"); }\n");
    fprintf(f, "static inline void _kofi_printf_nil(void* p) { (void)p; printf(\"nil\"); }\n\n");

    fprintf(f, "#define _kofi_printf_single(x) _Generic((x), \\\n");
    fprintf(f, "    bool: _kofi_printf_bln, \\\n");
    fprintf(f, "    char: _kofi_printf_int, \\\n");
    fprintf(f, "    short: _kofi_printf_int, \\\n");
    fprintf(f, "    int: _kofi_printf_int, \\\n");
    fprintf(f, "    long: _kofi_printf_int, \\\n");
    fprintf(f, "    long long: _kofi_printf_int, \\\n");
    fprintf(f, "    float: _kofi_printf_flt, \\\n");
    fprintf(f, "    double: _kofi_printf_flt, \\\n");
    fprintf(f, "    char*: _kofi_printf_stg, \\\n");
    fprintf(f, "    const char*: _kofi_printf_stg, \\\n");
    fprintf(f, "    default: _kofi_printf_nil \\\n");
    fprintf(f, ")(x)\n\n");

    // Input: scanf support
    fprintf(f, "static inline int _kofi_scan_int(const char* prompt, int* p) {\n");
    fprintf(f, "    if (prompt && prompt[0]) { printf(\"%%s\", prompt); fflush(stdout); }\n");
    fprintf(f, "    return scanf(\"%%d\", p) == 1;\n");
    fprintf(f, "}\n");

    fprintf(f, "static inline int _kofi_scan_int64(const char* prompt, long long* p) {\n");
    fprintf(f, "    if (prompt && prompt[0]) { printf(\"%%s\", prompt); fflush(stdout); }\n");
    fprintf(f, "    return scanf(\"%%lld\", p) == 1;\n");
    fprintf(f, "}\n");

    fprintf(f, "static inline int _kofi_scan_flt(const char* prompt, double* p) {\n");
    fprintf(f, "    if (prompt && prompt[0]) { printf(\"%%s\", prompt); fflush(stdout); }\n");
    fprintf(f, "    return scanf(\"%%lf\", p) == 1;\n");
    fprintf(f, "}\n");

    fprintf(f, "static inline int _kofi_scan_bln(const char* prompt, bool* p) {\n");
    fprintf(f, "    if (prompt && prompt[0]) { printf(\"%%s\", prompt); fflush(stdout); }\n");
    fprintf(f, "    char buf[32];\n");
    fprintf(f, "    if (scanf(\"%%31s\", buf) == 1) {\n");
    fprintf(f, "        *p = (strcmp(buf, \"true\") == 0 || strcmp(buf, \"1\") == 0);\n");
    fprintf(f, "        return 1;\n");
    fprintf(f, "    }\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");

    fprintf(f, "static inline int _kofi_scan_stg(const char* prompt, const char** p) {\n");
    fprintf(f, "    if (prompt && prompt[0]) { printf(\"%%s\", prompt); fflush(stdout); }\n");
    fprintf(f, "    char buf[1024];\n");
    fprintf(f, "    if (scanf(\" %%1023[^\\n]\", buf) == 1) {\n");
    fprintf(f, "        *p = strdup(buf);\n");
    fprintf(f, "        return 1;\n");
    fprintf(f, "    }\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");

    fprintf(f, "static inline int _kofi_scan_nil(const char* prompt, void** p) {\n");
    fprintf(f, "    if (prompt && prompt[0]) { printf(\"%%s\", prompt); fflush(stdout); }\n");
    fprintf(f, "    char buf[1024];\n");
    fprintf(f, "    if (scanf(\" %%1023[^\\n]\", buf) == 1) { (void)buf; }\n");
    fprintf(f, "    *p = NULL;\n");
    fprintf(f, "    return 1;\n");
    fprintf(f, "}\n\n");

    fprintf(f, "#define _kofi_scanf(prompt, ptr) _Generic((ptr), \\\n");
    fprintf(f, "    int*: _kofi_scan_int, \\\n");
    fprintf(f, "    long long*: _kofi_scan_int64, \\\n");
    fprintf(f, "    double*: _kofi_scan_flt, \\\n");
    fprintf(f, "    bool*: _kofi_scan_bln, \\\n");
    fprintf(f, "    const char**: _kofi_scan_stg, \\\n");
    fprintf(f, "    char**: (int(*)(const char*, char**))_kofi_scan_stg, \\\n");
    fprintf(f, "    void**: _kofi_scan_nil \\\n");
    fprintf(f, ")(prompt, ptr)\n\n");

    for (int i = 0; i < prog->func_count; i++) {
        Function fn = prog->functions[i];
        const char* ret = map_type(fn.ret_type);
        int is_main = strcmp(fn.name, "main") == 0;

        if (is_main && (!fn.ret_type || strcmp(fn.ret_type, "int") != 0)) {
            fprintf(f, "int main(");
        } else {
            fprintf(f, "%s %s(", ret, fn.name);
        }

        for (int j = 0; j < fn.param_count; j++) {
            fprintf(f, "%s %s", map_type(fn.params[j].type), fn.params[j].name);
            if (j < fn.param_count - 1) fprintf(f, ", ");
        }
        fprintf(f, ") {\n");

        for (int j = 0; j < fn.stmt_count; j++) {
            emit_stmt(f, &fn.stmts[j], 1);
        }

        if (is_main) {
            fprintf(f, "    return 0;\n");
        }
        fprintf(f, "}\n\n");
    }

    fclose(f);
}
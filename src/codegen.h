// src/codegen.h
#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"

void generate_code(Program* prog, const char* out_c_path);

#endif
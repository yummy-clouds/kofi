#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "parser.h"
#include "codegen.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: mylang <source.kf>\n");
        return 1;
    }

    const char* source_path = argv[1];
    char* source = read_file(source_path);

    // 1. Parse source code into AST
    Program* prog = parse(source);

    // 2. Generate C code
    const char* temp_c = "build/temp.c";
    generate_code(prog, temp_c);

    // 3. Invoke gcc to compile the generated C code into an executable binary
    char command[512];
    snprintf(command, sizeof(command), "gcc -std=c11 %s -o output", temp_c);
    int result = system(command);

    if (result != 0) {
        fprintf(stderr, "Compilation failed.\n");
        return 1;
    }

    printf("Successfully compiled! Run with ./output\n");

    free(source);
    return 0;
}
// src/util.c
#include "util.h"
#include <stdio.h>
#include <stdlib.h>

char* read_file(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "Error: Could not open file '%s'\n", path);
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = '\0';
    fclose(f);
    return buf;
}

void error(int line, const char* message) {
    fprintf(stderr, "[Line %d] Error: %s\n", line, message);
    exit(1);
}
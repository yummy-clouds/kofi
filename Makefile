CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

SRC = src/main.c src/lexer.c src/parser.c src/codegen.c src/util.c
OBJ = $(SRC:src/%.c=build/%.o)

ifeq ($(OS),Windows_NT)
    MKDIR = if not exist build mkdir build
    RM = if exist build rmdir /s /q build & if exist kofi.exe del /f /q kofi.exe & if exist output.exe del /f /q output.exe
    TARGET = kofi.exe
else
    MKDIR = mkdir -p build
    RM = rm -rf build kofi output
    TARGET = kofi
endif

all: builddir $(TARGET)

builddir:
	@$(MKDIR)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@$(RM)
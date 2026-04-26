#include <stdio.h>
#include <stdlib.h>
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/semantic.h"
#include "../include/codegen.h"

char* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(length + 1);
    if (buffer) {
        fread(buffer, 1, length, file);
    }
    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source.nv>\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];
    char* source_code = read_file(filename);

    printf("--- Compiling %s ---\n", filename);

    // 1. Parsing
    printf("[1/4] Parsing...\n");
    ASTNode* ast = parse_program(source_code);
    
    // 2. Semantic Analysis
    printf("[2/4] Semantic Analysis...\n");
    semantic_analyze(ast);
    
    // 3. Code Generation
    printf("[3/4] Generating LLVM IR...\n");
    codegen_init();
    codegen_program(ast);
    
    // Optional: Dump IR to console
    // codegen_dump_ir();
    
    // 4. Object File Emission
    printf("[4/4] Emitting Object File...\n");
    codegen_emit_object("output.o");
    
    // Cleanup
    codegen_cleanup();
    ast_free(ast);
    free(source_code);

    printf("Compilation successful! You can now link output.o with gcc or clang.\n");
    printf("Example: gcc output.o -o output.exe\n");

    return 0;
}

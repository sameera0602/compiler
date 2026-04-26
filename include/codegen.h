#ifndef CODEGEN_H
#define CODEGEN_H

#include "../include/ast.h"

void codegen_init();
void codegen_program(ASTNode* program);
void codegen_dump_ir();
void codegen_emit_object(const char* filename);
void codegen_cleanup();

#endif

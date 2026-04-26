#include <stdlib.h>
#include <stdio.h>
#include "../include/ast.h"

ASTNode* ast_create_node(ASTNodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    return node;
}

void ast_free(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_PROGRAM:
            for (int i = 0; i < node->data.program.func_count; i++) {
                ast_free(node->data.program.functions[i]);
            }
            free(node->data.program.functions);
            break;
        case AST_FUNCTION:
            free(node->data.function.name);
            for (int i = 0; i < node->data.function.param_count; i++) {
                ast_free(node->data.function.params[i]);
            }
            free(node->data.function.params);
            ast_free(node->data.function.body);
            break;
        case AST_PARAM:
            free(node->data.param.name);
            break;
        case AST_BLOCK:
            for (int i = 0; i < node->data.block.stmt_count; i++) {
                ast_free(node->data.block.statements[i]);
            }
            free(node->data.block.statements);
            break;
        case AST_LET:
            free(node->data.let_stmt.name);
            ast_free(node->data.let_stmt.initializer);
            break;
        case AST_RETURN:
            ast_free(node->data.return_stmt.value);
            break;
        case AST_IF:
            ast_free(node->data.if_stmt.condition);
            ast_free(node->data.if_stmt.then_branch);
            ast_free(node->data.if_stmt.else_branch);
            break;
        case AST_WHILE:
            ast_free(node->data.while_stmt.condition);
            ast_free(node->data.while_stmt.body);
            break;
        case AST_EXPR_STMT:
            ast_free(node->data.expr_stmt.expr);
            break;
        case AST_PRINT:
            ast_free(node->data.print_stmt.expr);
            break;
        case AST_BINOP:
            ast_free(node->data.binop.left);
            ast_free(node->data.binop.right);
            break;
        case AST_IDENTIFIER:
            free(node->data.identifier.name);
            break;
        case AST_CALL:
            free(node->data.call.callee);
            for (int i = 0; i < node->data.call.arg_count; i++) {
                ast_free(node->data.call.args[i]);
            }
            free(node->data.call.args);
            break;
        case AST_NUMBER:
            break;
    }
    
    free(node);
}

void ast_print(ASTNode* node, int indent) {
    if (!node) return;
    for (int i = 0; i < indent; i++) printf("  ");
    
    switch (node->type) {
        case AST_PROGRAM:
            printf("Program\n");
            for (int i = 0; i < node->data.program.func_count; i++) {
                ast_print(node->data.program.functions[i], indent + 1);
            }
            break;
        case AST_FUNCTION:
            printf("Function: %s -> %s\n", node->data.function.name, node->data.function.return_type == TYPE_INT ? "int" : "void");
            for (int i = 0; i < node->data.function.param_count; i++) {
                ast_print(node->data.function.params[i], indent + 1);
            }
            ast_print(node->data.function.body, indent + 1);
            break;
        // ... omitted full ast_print implementation for brevity, typically used for debugging ...
        default:
            printf("Node(%d)\n", node->type);
            break;
    }
}

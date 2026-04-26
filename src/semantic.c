#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/semantic.h"

// Simple Symbol Table (Linked List for demonstration)
typedef struct Symbol {
    char* name;
    DataType type;
    bool is_function;
    struct Symbol* next;
} Symbol;

static Symbol* symbol_table = NULL;

static void push_symbol(char* name, DataType type, bool is_function) {
    Symbol* sym = (Symbol*)malloc(sizeof(Symbol));
    sym->name = strdup(name);
    sym->type = type;
    sym->is_function = is_function;
    sym->next = symbol_table;
    symbol_table = sym;
}

static Symbol* find_symbol(char* name) {
    Symbol* current = symbol_table;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static void free_symbols() {
    Symbol* current = symbol_table;
    while (current != NULL) {
        Symbol* next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
    symbol_table = NULL;
}

// Forward declarations
static DataType analyze_expr(ASTNode* node);
static void analyze_stmt(ASTNode* node);

static DataType analyze_expr(ASTNode* node) {
    if (!node) return TYPE_VOID;
    
    switch (node->type) {
        case AST_NUMBER:
            return TYPE_INT;
        case AST_IDENTIFIER: {
            Symbol* sym = find_symbol(node->data.identifier.name);
            if (!sym) {
                fprintf(stderr, "Semantic Error: Undefined variable '%s'\n", node->data.identifier.name);
                exit(1);
            }
            return sym->type;
        }
        case AST_BINOP: {
            DataType left = analyze_expr(node->data.binop.left);
            DataType right = analyze_expr(node->data.binop.right);
            if (left != right) {
                fprintf(stderr, "Semantic Error: Type mismatch in binary operation\n");
                exit(1);
            }
            return left; // Assuming only int operations for now
        }
        case AST_CALL: {
            Symbol* sym = find_symbol(node->data.call.callee);
            if (!sym || !sym->is_function) {
                fprintf(stderr, "Semantic Error: Undefined function '%s'\n", node->data.call.callee);
                exit(1);
            }
            // Realistically we should check argument types and counts here
            return sym->type;
        }
        default:
            return TYPE_VOID;
    }
}

static void analyze_stmt(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_LET:
            if (find_symbol(node->data.let_stmt.name)) {
                fprintf(stderr, "Semantic Error: Variable '%s' already declared in this scope\n", node->data.let_stmt.name);
                exit(1);
            }
            DataType init_type = analyze_expr(node->data.let_stmt.initializer);
            if (init_type != node->data.let_stmt.var_type) {
                fprintf(stderr, "Semantic Error: Type mismatch in variable initialization '%s'\n", node->data.let_stmt.name);
                exit(1);
            }
            push_symbol(node->data.let_stmt.name, node->data.let_stmt.var_type, false);
            break;
        case AST_RETURN:
            analyze_expr(node->data.return_stmt.value);
            // Note: Should check if return type matches function's declared return type
            break;
        case AST_IF:
            analyze_expr(node->data.if_stmt.condition);
            analyze_stmt(node->data.if_stmt.then_branch);
            if (node->data.if_stmt.else_branch) {
                analyze_stmt(node->data.if_stmt.else_branch);
            }
            break;
        case AST_WHILE:
            analyze_expr(node->data.while_stmt.condition);
            analyze_stmt(node->data.while_stmt.body);
            break;
        case AST_EXPR_STMT:
            analyze_expr(node->data.expr_stmt.expr);
            break;
        case AST_PRINT:
            analyze_expr(node->data.print_stmt.expr);
            break;
        case AST_BLOCK: {
            // Simplified scoping: a real compiler pushes/pops a scope environment here
            for (int i = 0; i < node->data.block.stmt_count; i++) {
                analyze_stmt(node->data.block.statements[i]);
            }
            break;
        }
        default:
            break;
    }
}

static void analyze_function(ASTNode* node) {
    push_symbol(node->data.function.name, node->data.function.return_type, true);
    
    // In a real compiler, we would push a new scope here
    for (int i = 0; i < node->data.function.param_count; i++) {
        ASTNode* param = node->data.function.params[i];
        push_symbol(param->data.param.name, param->data.param.var_type, false);
    }
    
    analyze_stmt(node->data.function.body);
    // Pop scope...
}

void semantic_analyze(ASTNode* program) {
    if (program->type != AST_PROGRAM) return;
    
    for (int i = 0; i < program->data.program.func_count; i++) {
        // Pre-declare functions
        ASTNode* func = program->data.program.functions[i];
        push_symbol(func->data.function.name, func->data.function.return_type, true);
    }
    
    for (int i = 0; i < program->data.program.func_count; i++) {
        analyze_function(program->data.program.functions[i]);
    }
    
    free_symbols();
}

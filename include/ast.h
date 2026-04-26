#ifndef AST_H
#define AST_H

#include <stdbool.h>

typedef enum {
    AST_PROGRAM,
    AST_FUNCTION,
    AST_PARAM,
    AST_BLOCK,
    AST_LET,
    AST_RETURN,
    AST_IF,
    AST_WHILE,
    AST_EXPR_STMT,
    AST_PRINT,
    AST_BINOP,
    AST_NUMBER,
    AST_IDENTIFIER,
    AST_CALL
} ASTNodeType;

typedef enum {
    OP_ADD, OP_SUB, OP_MUL, OP_DIV,
    OP_EQ, OP_NEQ, OP_LT, OP_GT, OP_LTE, OP_GTE, OP_ASSIGN
} OperatorType;

typedef enum {
    TYPE_INT,
    TYPE_VOID
} DataType;

typedef struct ASTNode {
    ASTNodeType type;
    
    // Union to hold specific node data
    union {
        // Program
        struct {
            struct ASTNode** functions;
            int func_count;
        } program;
        
        // Function
        struct {
            char* name;
            struct ASTNode** params;
            int param_count;
            DataType return_type;
            struct ASTNode* body;
        } function;
        
        // Parameter
        struct {
            char* name;
            DataType var_type;
        } param;
        
        // Block
        struct {
            struct ASTNode** statements;
            int stmt_count;
        } block;
        
        // Let (Variable declaration)
        struct {
            char* name;
            DataType var_type;
            struct ASTNode* initializer;
        } let_stmt;
        
        // Return
        struct {
            struct ASTNode* value;
        } return_stmt;
        
        // If
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_branch;
            struct ASTNode* else_branch; // optional
        } if_stmt;
        
        // While
        struct {
            struct ASTNode* condition;
            struct ASTNode* body;
        } while_stmt;
        
        // ExprStmt
        struct {
            struct ASTNode* expr;
        } expr_stmt;
        
        // Print
        struct {
            struct ASTNode* expr;
        } print_stmt;
        
        // BinOp
        struct {
            OperatorType op;
            struct ASTNode* left;
            struct ASTNode* right;
        } binop;
        
        // Number
        struct {
            int value;
        } number;
        
        // Identifier
        struct {
            char* name;
        } identifier;
        
        // Call
        struct {
            char* callee;
            struct ASTNode** args;
            int arg_count;
        } call;
    } data;
} ASTNode;

ASTNode* ast_create_node(ASTNodeType type);
void ast_free(ASTNode* node);
void ast_print(ASTNode* node, int indent);

#endif

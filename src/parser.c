#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/parser.h"
#include "../include/lexer.h"

static Token current_token;

static void next_token() {
    lexer_free_token(&current_token);
    current_token = lexer_next_token();
}

static void expect(TokenType type) {
    if (current_token.type == type) {
        next_token();
    } else {
        fprintf(stderr, "Syntax Error at line %d: Expected token %d but got %d ('%s')\n", 
                current_token.line, type, current_token.type, current_token.text);
        exit(1);
    }
}

// Forward declarations
static ASTNode* parse_expression();
static ASTNode* parse_statement();
static ASTNode* parse_block();

static ASTNode* parse_primary() {
    ASTNode* node = NULL;
    if (current_token.type == TOK_NUMBER) {
        node = ast_create_node(AST_NUMBER);
        node->data.number.value = atoi(current_token.text);
        next_token();
    } else if (current_token.type == TOK_IDENTIFIER) {
        char* name = strdup(current_token.text);
        next_token();
        if (current_token.type == TOK_LPAREN) {
            // Function call
            next_token();
            node = ast_create_node(AST_CALL);
            node->data.call.callee = name;
            node->data.call.arg_count = 0;
            node->data.call.args = malloc(sizeof(ASTNode*) * 10); // simplified sizing
            
            if (current_token.type != TOK_RPAREN) {
                do {
                    node->data.call.args[node->data.call.arg_count++] = parse_expression();
                    if (current_token.type == TOK_COMMA) next_token();
                    else break;
                } while (1);
            }
            expect(TOK_RPAREN);
        } else {
            node = ast_create_node(AST_IDENTIFIER);
            node->data.identifier.name = name;
        }
    } else if (current_token.type == TOK_LPAREN) {
        next_token();
        node = parse_expression();
        expect(TOK_RPAREN);
    } else {
        fprintf(stderr, "Syntax Error at line %d: Unexpected token '%s'\n", current_token.line, current_token.text);
        exit(1);
    }
    return node;
}

static ASTNode* parse_term() {
    ASTNode* left = parse_primary();
    while (current_token.type == TOK_STAR || current_token.type == TOK_SLASH) {
        OperatorType op = current_token.type == TOK_STAR ? OP_MUL : OP_DIV;
        next_token();
        ASTNode* right = parse_primary();
        ASTNode* node = ast_create_node(AST_BINOP);
        node->data.binop.op = op;
        node->data.binop.left = left;
        node->data.binop.right = right;
        left = node;
    }
    return left;
}

static ASTNode* parse_additive() {
    ASTNode* left = parse_term();
    while (current_token.type == TOK_PLUS || current_token.type == TOK_MINUS) {
        OperatorType op = current_token.type == TOK_PLUS ? OP_ADD : OP_SUB;
        next_token();
        ASTNode* right = parse_term();
        ASTNode* node = ast_create_node(AST_BINOP);
        node->data.binop.op = op;
        node->data.binop.left = left;
        node->data.binop.right = right;
        left = node;
    }
    return left;
}

static ASTNode* parse_comparison() {
    ASTNode* left = parse_additive();
    if (current_token.type == TOK_EQ || current_token.type == TOK_NEQ ||
        current_token.type == TOK_LT || current_token.type == TOK_GT ||
        current_token.type == TOK_LTE || current_token.type == TOK_GTE) {
        OperatorType op;
        switch (current_token.type) {
            case TOK_EQ: op = OP_EQ; break;
            case TOK_NEQ: op = OP_NEQ; break;
            case TOK_LT: op = OP_LT; break;
            case TOK_GT: op = OP_GT; break;
            case TOK_LTE: op = OP_LTE; break;
            case TOK_GTE: op = OP_GTE; break;
            default: exit(1);
        }
        next_token();
        ASTNode* right = parse_additive();
        ASTNode* node = ast_create_node(AST_BINOP);
        node->data.binop.op = op;
        node->data.binop.left = left;
        node->data.binop.right = right;
        left = node;
    }
    return left;
}

static ASTNode* parse_expression() {
    return parse_comparison();
}

static ASTNode* parse_statement() {
    ASTNode* node = NULL;
    if (current_token.type == TOK_LET) {
        next_token();
        node = ast_create_node(AST_LET);
        node->data.let_stmt.name = strdup(current_token.text);
        expect(TOK_IDENTIFIER);
        expect(TOK_COLON);
        if (current_token.type == TOK_INT) {
            node->data.let_stmt.var_type = TYPE_INT;
            next_token();
        } else {
            fprintf(stderr, "Syntax Error: Unsupported type\n");
            exit(1);
        }
        expect(TOK_ASSIGN);
        node->data.let_stmt.initializer = parse_expression();
        expect(TOK_SEMICOLON);
    } else if (current_token.type == TOK_RETURN) {
        next_token();
        node = ast_create_node(AST_RETURN);
        node->data.return_stmt.value = parse_expression();
        expect(TOK_SEMICOLON);
    } else if (current_token.type == TOK_IF) {
        next_token();
        node = ast_create_node(AST_IF);
        expect(TOK_LPAREN);
        node->data.if_stmt.condition = parse_expression();
        expect(TOK_RPAREN);
        node->data.if_stmt.then_branch = parse_block();
        if (current_token.type == TOK_ELSE) {
            next_token();
            node->data.if_stmt.else_branch = parse_block();
        } else {
            node->data.if_stmt.else_branch = NULL;
        }
    } else if (current_token.type == TOK_PRINT) {
        next_token();
        expect(TOK_LPAREN);
        node = ast_create_node(AST_PRINT);
        node->data.print_stmt.expr = parse_expression();
        expect(TOK_RPAREN);
        expect(TOK_SEMICOLON);
    } else {
        node = ast_create_node(AST_EXPR_STMT);
        node->data.expr_stmt.expr = parse_expression();
        expect(TOK_SEMICOLON);
    }
    return node;
}

static ASTNode* parse_block() {
    expect(TOK_LBRACE);
    ASTNode* node = ast_create_node(AST_BLOCK);
    node->data.block.stmt_count = 0;
    node->data.block.statements = malloc(sizeof(ASTNode*) * 100);
    
    while (current_token.type != TOK_RBRACE && current_token.type != TOK_EOF) {
        node->data.block.statements[node->data.block.stmt_count++] = parse_statement();
    }
    expect(TOK_RBRACE);
    return node;
}

static ASTNode* parse_function() {
    expect(TOK_FN);
    ASTNode* node = ast_create_node(AST_FUNCTION);
    node->data.function.name = strdup(current_token.text);
    expect(TOK_IDENTIFIER);
    
    expect(TOK_LPAREN);
    node->data.function.param_count = 0;
    node->data.function.params = malloc(sizeof(ASTNode*) * 10);
    
    if (current_token.type != TOK_RPAREN) {
        do {
            ASTNode* param = ast_create_node(AST_PARAM);
            param->data.param.name = strdup(current_token.text);
            expect(TOK_IDENTIFIER);
            expect(TOK_COLON);
            if (current_token.type == TOK_INT) {
                param->data.param.var_type = TYPE_INT;
                next_token();
            }
            node->data.function.params[node->data.function.param_count++] = param;
            
            if (current_token.type == TOK_COMMA) next_token();
            else break;
        } while (1);
    }
    expect(TOK_RPAREN);
    
    expect(TOK_ARROW);
    if (current_token.type == TOK_INT) {
        node->data.function.return_type = TYPE_INT;
        next_token();
    } else if (current_token.type == TOK_VOID) {
        node->data.function.return_type = TYPE_VOID;
        next_token();
    } else {
        node->data.function.return_type = TYPE_VOID; // Default or error
    }
    
    node->data.function.body = parse_block();
    return node;
}

ASTNode* parse_program(const char* source_code) {
    lexer_init(source_code);
    current_token.text = NULL;
    next_token();
    
    ASTNode* node = ast_create_node(AST_PROGRAM);
    node->data.program.func_count = 0;
    node->data.program.functions = malloc(sizeof(ASTNode*) * 50);
    
    while (current_token.type != TOK_EOF) {
        node->data.program.functions[node->data.program.func_count++] = parse_function();
    }
    
    return node;
}

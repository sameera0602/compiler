#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOK_EOF = 0,
    TOK_FN, TOK_LET, TOK_RETURN, TOK_IF, TOK_ELSE, TOK_WHILE, TOK_PRINT,
    TOK_INT, TOK_FLOAT, TOK_VOID,
    TOK_IDENTIFIER, TOK_NUMBER,
    TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH,
    TOK_ASSIGN, TOK_EQ, TOK_NEQ, TOK_LT, TOK_GT, TOK_LTE, TOK_GTE,
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, TOK_RBRACE,
    TOK_COMMA, TOK_COLON, TOK_ARROW, TOK_SEMICOLON,
    TOK_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char* text;
    int line;
    int column;
} Token;

void lexer_init(const char* source_code);
Token lexer_next_token();
void lexer_free_token(Token* token);

#endif

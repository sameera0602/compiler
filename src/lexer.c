#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/lexer.h"

static const char* src;
static int pos = 0;
static int line = 1;
static int col = 1;

void lexer_init(const char* source_code) {
    src = source_code;
    pos = 0;
    line = 1;
    col = 1;
}

static char peek() {
    return src[pos];
}

static char advance() {
    char c = src[pos++];
    if (c == '\n') {
        line++;
        col = 1;
    } else {
        col++;
    }
    return c;
}

static void skip_whitespace() {
    while (isspace(peek())) {
        advance();
    }
}

static Token make_token(TokenType type, const char* start, int length) {
    Token t;
    t.type = type;
    t.line = line;
    t.column = col - length;
    t.text = (char*)malloc(length + 1);
    strncpy(t.text, start, length);
    t.text[length] = '\0';
    return t;
}

Token lexer_next_token() {
    skip_whitespace();

    if (peek() == '\0') {
        return make_token(TOK_EOF, "EOF", 3);
    }

    char c = peek();
    const char* start = &src[pos];

    if (isalpha(c) || c == '_') {
        int length = 0;
        while (isalnum(peek()) || peek() == '_') {
            advance();
            length++;
        }
        
        // Check keywords
        if (strncmp(start, "fn", length) == 0 && length == 2) return make_token(TOK_FN, start, length);
        if (strncmp(start, "let", length) == 0 && length == 3) return make_token(TOK_LET, start, length);
        if (strncmp(start, "return", length) == 0 && length == 6) return make_token(TOK_RETURN, start, length);
        if (strncmp(start, "if", length) == 0 && length == 2) return make_token(TOK_IF, start, length);
        if (strncmp(start, "else", length) == 0 && length == 4) return make_token(TOK_ELSE, start, length);
        if (strncmp(start, "while", length) == 0 && length == 5) return make_token(TOK_WHILE, start, length);
        if (strncmp(start, "print", length) == 0 && length == 5) return make_token(TOK_PRINT, start, length);
        if (strncmp(start, "int", length) == 0 && length == 3) return make_token(TOK_INT, start, length);
        
        return make_token(TOK_IDENTIFIER, start, length);
    }

    if (isdigit(c)) {
        int length = 0;
        while (isdigit(peek())) {
            advance();
            length++;
        }
        return make_token(TOK_NUMBER, start, length);
    }

    advance();
    switch (c) {
        case '+': return make_token(TOK_PLUS, start, 1);
        case '-': 
            if (peek() == '>') {
                advance();
                return make_token(TOK_ARROW, start, 2);
            }
            return make_token(TOK_MINUS, start, 1);
        case '*': return make_token(TOK_STAR, start, 1);
        case '/': return make_token(TOK_SLASH, start, 1);
        case '=':
            if (peek() == '=') {
                advance();
                return make_token(TOK_EQ, start, 2);
            }
            return make_token(TOK_ASSIGN, start, 1);
        case '!':
            if (peek() == '=') {
                advance();
                return make_token(TOK_NEQ, start, 2);
            }
            break;
        case '<':
            if (peek() == '=') {
                advance();
                return make_token(TOK_LTE, start, 2);
            }
            return make_token(TOK_LT, start, 1);
        case '>':
            if (peek() == '=') {
                advance();
                return make_token(TOK_GTE, start, 2);
            }
            return make_token(TOK_GT, start, 1);
        case '(': return make_token(TOK_LPAREN, start, 1);
        case ')': return make_token(TOK_RPAREN, start, 1);
        case '{': return make_token(TOK_LBRACE, start, 1);
        case '}': return make_token(TOK_RBRACE, start, 1);
        case ',': return make_token(TOK_COMMA, start, 1);
        case ':': return make_token(TOK_COLON, start, 1);
        case ';': return make_token(TOK_SEMICOLON, start, 1);
    }

    return make_token(TOK_UNKNOWN, start, 1);
}

void lexer_free_token(Token* token) {
    if (token->text) {
        free(token->text);
        token->text = NULL;
    }
}

#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "arena.h"

#include <stdint.h>

typedef enum {
    ET_NUMBER,
    ET_BINARY
} ExprType;

typedef struct Expr {
    ExprType type;
    union {
        uint64_t number;
        struct {
            struct Expr* left;
            char op;
            struct Expr* right;
        } binary;
    } as;
} Expr;

typedef enum {
    ST_RETURN,
} StatementType;

typedef struct {
    StatementType type;
    union {
        Expr* ret;
    } as;
} Statement;

typedef struct {
    Token* tokens;
    size_t pos;
    Statement* statements;
    Arena* arena;
} Parser;

bool parser_is_finished(Parser* parser); 
Token parser_peek(const Parser* parser);
Token parser_next(Parser* parser);
Expr* parser_primary(Parser* parser);
Expr* parser_expr(Parser* parser, int min_prec);
Statement parser_statement(Parser* parser);
int parser_current_token_precedence(const Parser* parser);

#endif

#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "arena.h"

#include <stdint.h>

typedef enum {
    ET_NUMBER,
    ET_VARIABLE,
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
        char* variable;
    } as;
} Expr;

typedef enum {
    // return <expr>;
    ST_RETURN,
    // let/const <name>: <type> = <expr>;
    ST_VARIABLE_DEFINE,
    ST_ERROR
} StatementType;

typedef struct {
    StatementType type;
    union {
        Expr* ret;
        struct {
            char* name;
            char* type;
            Expr* value;
        } var_def;
    } as;
} Statement;

typedef struct {
    const char* message;
    Location loc;
} ParserError;

typedef struct {
    char* token_origin;
    Token* tokens;
    ptrdiff_t pos;
    Statement* statements;
    Arena* arena;
    ParserError error;
} Parser;

bool parser_is_finished(Parser* parser); 
Token parser_peek(const Parser* parser);
Token parser_next(Parser* parser);
Expr* parser_primary(Parser* parser);
Expr* parser_expr(Parser* parser, int min_prec);
bool parser_statement(Parser* parser);
int parser_current_token_precedence(const Parser* parser);
void parser_error_display(ParserError error, char* file_content, char* input_name);

#endif

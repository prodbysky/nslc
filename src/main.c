#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>

#define STB_DS_IMPLEMENTATION
#include "../extern/stb_ds.h"

#include "lexer.h"
#include "arena.h"

// Returns a null terminaed string of the file in `name`
// On any error (except malloc) prints error with perror and returns NULL
char* read_file(const char* name);
Lexer lex_file(char* content, Arena* arena);

const char* TokenTypeReadable[TT_COUNT] = {
    [TT_NUMBER] = "Number",
    [TT_OPERATOR] = "Operator",
};

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

typedef struct {
    Token* tokens;
    size_t pos;
    Expr* exprs;
    Arena* arena;
} Parser;

bool parser_is_finished(Parser* parser); 
Token parser_peek(const Parser* parser);
Token parser_next(Parser* parser);
Expr* parser_primary(Parser* parser);
Expr* parser_expr(Parser* parser, int min_prec);
int parser_current_token_precedence(const Parser* parser);

int main() {
    char* file_content = read_file("test.nsl");

    Arena arena = arena_new(1024 * 10);

    Lexer lexer = lex_file(file_content, &arena);
    // for (ptrdiff_t i = 0; i < arrlen(tokens); i++) token_print(tokens[i]);

    Parser parser = {
        .tokens = lexer.tokens,
        .pos = 0,
        .exprs = NULL,
        .arena = &arena
    };

    while (!parser_is_finished(&parser)) {
        Expr* expr = parser_expr(&parser, 0);
        printf("left: %lu\n", expr->as.binary.left->as.number);
        printf("op: %c\n", expr->as.binary.op);
        printf("right: %lu\n", expr->as.binary.right->as.number);
    }

    free(file_content);
    arrfree(lexer.tokens);
    arena_delete(&arena);

	return 0;
}

bool parser_is_finished(Parser* parser) {
    return arrlen(parser->tokens) <= parser->pos;
}

Token parser_peek(const Parser* parser) {
    return parser->tokens[parser->pos];
}

Token parser_next(Parser* parser) {
    return parser->tokens[parser->pos++];
}
int parser_current_token_precedence(const Parser* parser) {
    switch (parser_peek(parser).as.operator) {
        case '+': case '-': return 1;
        case '*': case '/': return 2;
    }
    return -1;
}

Expr* parser_primary(Parser* parser) {
    Token t = parser_peek(parser);
    switch (t.type) {
        case TT_NUMBER: {
            Expr* expr = arena_alloc(parser->arena, sizeof(Expr));
            expr->type = ET_NUMBER;
            expr->as.number = t.as.number;
            parser_next(parser);
            return expr;
        }
        case TT_COUNT: {
            assert(false && "Unreachable :)");
        }
        default: {
            fprintf(stderr, "Unexpected token found when parsing primary expression: %s", TokenTypeReadable[t.type]);
            return NULL;
        }
    }
}
Expr* parser_expr(Parser* parser, int min_prec) {
    Expr* left = parser_primary(parser);

    while (!parser_is_finished(parser)) {
        Token t = parser_peek(parser);
        if (t.type == TT_OPERATOR) {
            int prec = parser_current_token_precedence(parser);
            if (prec < min_prec) break;
            char op = parser_peek(parser).as.operator;

            parser_next(parser);
            Expr* right = parser_expr(parser, prec + 1);
            Expr* new_left = arena_alloc(parser->arena, sizeof(Expr));

            *new_left = (Expr) {
                .type = ET_BINARY,
                .as = {
                    .binary = {
                        .left = left,
                        .op = op,
                        .right = right
                    }
                },
            };
            left = new_left;
        } else {
            break;
        }
    }

    return left;
}

Lexer lex_file(char* content, Arena* arena) {
    Lexer lexer = {
        .source = {
            .first = content,
            .current = content,
            .loc = {.col = 1, .row = 1},
        },
        .tokens = NULL, 
        .arena = arena
    };
    
    while (!lexer_is_finished(&lexer)) {
        lexer_skip_ws(&lexer);
        if (lexer_is_finished(&lexer)) break;
        if (!lexer_parse_token(&lexer)) {
            arrfree(lexer.tokens);
            return (Lexer) {};
        }
    }
    return lexer;
}


char* read_file(const char* name) {
    FILE* file = fopen(name, "r");
    if (file == NULL) {
        perror("failed to input file");
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    size_t len = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = malloc(sizeof(char) * (len + 1));
    assert(buffer);
    buffer[len] = 0;

    fread(buffer, sizeof(char), len, file);

    fclose(file);
    return buffer;

}

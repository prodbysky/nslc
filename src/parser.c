#include "parser.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../extern/stb_ds.h"

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

bool parser_expect(Parser* parser, TokenType t, char* err_msg) {
    if (parser_is_finished(parser) || parser_peek(parser).type != t) {
        fprintf(stderr, "%s\n", err_msg);
        return false;
    }
    return true;
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
        case TT_OPENPAREN: {
            parser_next(parser);
            Expr* expr = parser_expr(parser, 0);
            if (expr == NULL) {
                fprintf(stderr, "Failed to parse parenthesized expression value\n");
                return false;
            }
            parser_next(parser);
            return expr;
        }
        case TT_IDENT: {
            Expr* expr = arena_alloc(parser->arena, sizeof(Expr));
            expr->type = ET_VARIABLE;
            expr->as.variable = t.as.ident;
            if (expr == NULL) {
                fprintf(stderr, "Failed to parse variable name expression value\n");
                return false;
            }
            parser_next(parser);
            return expr;
        }
        case TT_COUNT: {
            assert(false && "Unreachable :)");
        }
        default: {
            fprintf(stderr, "Unexpected token found when parsing primary expression\n");
            token_print(t);
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
            if (right == NULL) return NULL;
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

bool parser_statement(Parser* parser) {
    if (parser_is_finished(parser)) {
        fprintf(stderr, "Unexpected EOF\n");
        return false;
    }
    Token t = parser_peek(parser);
    switch (t.type) {
        case TT_KEYWORD: {
            if (t.as.keyword == TK_RETURN) {
                parser_next(parser);
                Expr* value = parser_expr(parser, 0);
                if (value == NULL) {
                    fprintf(stderr, "Failed to parse return statement value\n");
                    return false;
                }
                
                if (!parser_expect(parser, TT_SEMICOLON, "Expected semicolon after return statement")) return false;
                parser_next(parser);
                
                Statement st = {
                    .type = ST_RETURN,
                    .as = {
                        .ret = value
                    },
                };
                arrput(parser->statements, st);
                return true;
            }
            if (t.as.keyword == TK_LET) {
                parser_next(parser); 
                if (!parser_expect(parser, TT_IDENT, "Expected identifier after let")) return false;
                Token name = parser_next(parser);
                
                if (!parser_expect(parser, TT_COLON, "Expected colon after variable name")) return false;
                parser_next(parser);
                
                if (!parser_expect(parser, TT_IDENT, "Expected type after colon")) return false;
                Token type = parser_next(parser);
                
                if (!parser_expect(parser, TT_EQUAL, "Expected equals sign after type")) return false;
                parser_next(parser);
                
                Expr* expr = parser_expr(parser, 0);
                if (expr == NULL) {
                    fprintf(stderr, "Failed to parse variable definition value expression\n");
                    return false;
                }
                
                if (!parser_expect(parser, TT_SEMICOLON, "Expected semicolon after variable definition")) return false;
                parser_next(parser); 
                
                Statement st = {
                    .type = ST_VARIABLE_DEFINE,
                    .as.var_def = {
                        .name = name.as.ident,
                        .type = type.as.ident,
                        .value = expr
                    }
                };  
                arrput(parser->statements, st);
                return true;
            }
            break;
        }
        default: {
            Token t = parser_peek(parser);
            fprintf(stderr, "type: %u\n", t.type);
            fprintf(stderr, "Unexpected token type when trying to parse statement\n");
            return false;
        }
    }
    return false;
}

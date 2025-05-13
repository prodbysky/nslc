#include "parser.h"
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include "../extern/stb_ds.h"
#include "arena.h"
#include "lexer.h"

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
        case '>': case '<': return 1;
        case '+': case '-': return 2;
        case '*': case '/': return 3;
    }
    return -1;
}

bool parser_expect(Parser* parser, TokenType t, char* err_msg) {
    if (parser_is_finished(parser) || parser_peek(parser).type != t) {
        fprintf(stderr, "[parser::error] %s:%lu:%lu: %s\n", parser->token_origin, parser_peek(parser).loc.row, parser_peek(parser).loc.col, err_msg);
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
            if (expr == NULL) {
                fprintf(stderr, "Failed to parse variable name expression value\n");
                return false;
            }
            expr->type = ET_VARIABLE;
            expr->as.variable = t.as.ident;
            parser_next(parser);
            return expr;
        }
        case TT_KEYWORD: {
            if (t.as.keyword != TK_FALSE && t.as.keyword != TK_TRUE) {
                fprintf(stderr, "Something really wrong happened, tried to parse a keyword that's not `true` or `false`\n");
                return false;
            }
            Expr* expr = arena_alloc(parser->arena, sizeof(Expr));
            expr->type = ET_BOOL;
            expr->as.boolean = t.as.keyword == TK_TRUE;
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

bool parser_statement(Parser* parser, Statement** statements) {
    if (parser_is_finished(parser)) {
        fprintf(stderr, "Unexpected EOF\n");
        return false;
    }
    Token t = parser_peek(parser);
    switch (t.type) {
        case TT_KEYWORD: {
            if (t.as.keyword == TK_RETURN) { if (!parser_return_statement(parser, statements)) { return false; } return true; }
            if (t.as.keyword == TK_LET) {if (!parser_let_statement(parser, statements)) { return false; } return true; }
            if (t.as.keyword == TK_IF) {if (!parser_if_statement(parser, statements)) { return false; } return true; }
            if (t.as.keyword == TK_WHILE) {if (!parser_while_statement(parser, statements)) { return false; } return true; }
            break;
        }
        case TT_IDENT: {
            if (!parser_expect(parser, TT_IDENT, "Expected name in variable assignment statement\n")) return false;
            Token t_ident = parser_next(parser);
            Location begin = t_ident.loc;
            char* var_name = t_ident.as.ident;
            if (!parser_expect(parser, TT_EQUAL, "Expected name in variable assignment statement\n")) return false;
            parser_next(parser);
            Expr* new_value = parser_expr(parser, 0);
            if (new_value == NULL) {
                fprintf(stderr, "Failed to parse new value expression\n");
                return false;
            }
            if (!parser_expect(parser, TT_SEMICOLON, "Expected `;` after new value expression in variable assignment statement\n")) return false;
            parser_next(parser);
            Statement st = {
                .type = ST_SET_VARIABLE,
                .loc = begin,
                .as.var_assign = {
                    .new_val = new_value,
                    .var = var_name,
                }
            };
            arrput(*statements, st);
            return true;
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

bool parser_let_statement(Parser* parser, Statement** statements) {
    Location loc = parser_next(parser).loc; 
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
        .loc = loc,
        .as.var_def = {
            .name = name.as.ident,
            .type = type.as.ident,
            .value = expr
        }
    };  
    arrput(*statements, st);
    return true;
}
bool parser_return_statement(Parser* parser, Statement** statements) {
    Location loc = parser_next(parser).loc;
    Expr* value = parser_expr(parser, 0);
    if (value == NULL) {
        fprintf(stderr, "Failed to parse return statement value\n");
        return false;
    }

    if (!parser_expect(parser, TT_SEMICOLON, "Expected semicolon after return statement")) return false;
    parser_next(parser);

    Statement st = {
        .type = ST_RETURN,
        .loc = loc,
        .as = {
            .ret = value
        },
    };
    arrput(*statements, st);
    return true;
}

bool parser_if_statement(Parser* parser, Statement** statements) {
    Location loc = parser_next(parser).loc;
    Expr* value = parser_expr(parser, 0);
    if (value == NULL) {
        fprintf(stderr, "Failed to parse if statement condition...\n");
        return false;
    }
    if (!parser_expect(parser, TT_OPENCURLY, "Expected { after if condition expression")) return false;
    parser_next(parser);

    Statement* sts = NULL;
    while (!parser_expect(parser, TT_CLOSECURLY, "Skipping")) {
        if (!parser_statement(parser, &sts)) {
            fprintf(stderr, "Failed to parse if body due to invalid statement\n");
            return false;
        }
    }
    parser_next(parser);
    Statement st = {
        .loc = loc,
        .type = ST_IF,
        .as.if_st ={
            .body = sts,
            .cond = value
        }
    };
    arrput(*statements, st);

    return true;
}

bool parser_while_statement(Parser* parser, Statement** statements) {
    Location loc = parser_next(parser).loc;
    Expr* value = parser_expr(parser, 0);
    if (value == NULL) {
        fprintf(stderr, "Failed to parse while statement condition...\n");
        return false;
    }
    if (!parser_expect(parser, TT_OPENCURLY, "Expected { after while condition expression")) return false;
    parser_next(parser);

    Statement* sts = NULL;
    while (!parser_expect(parser, TT_CLOSECURLY, "Skipping")) {
        if (!parser_statement(parser, &sts)) {
            fprintf(stderr, "Failed to parse while body due to invalid statement\n");
            return false;
        }
    }
    parser_next(parser);
    Statement st = {
        .type = ST_WHILE,
        .loc = loc,
        .as.while_st = {
            .body = sts,
            .cond = value
        }
    };
    arrput(*statements, st);

    return true;
}

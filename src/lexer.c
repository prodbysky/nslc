#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>

#include "lexer.h"

#include "../extern/stb_ds.h"

bool lexer_is_finished(const Lexer* lexer) {
    return *lexer->source.current == 0;
}

char lexer_next(Lexer* lexer) {
    if (*lexer->source.current == '\n') {
        lexer->source.loc.col++;
        lexer->source.loc.row = 1;
    } else {
        lexer->source.loc.row++;
    }
    return *(lexer->source.current++);
}
char lexer_peek(const Lexer* lexer) {
    return *(lexer->source.current);
}

const char* lexer_skip_while(Lexer* lexer, int (*pred)(int)) {
    while (pred(lexer_peek(lexer)) && !lexer_is_finished(lexer)) lexer_next(lexer);
    return lexer->source.current;
}

const char* lexer_skip_ws(Lexer* lexer) {
    return lexer_skip_while(lexer, isspace);
}

bool lexer_parse_token(Lexer* lexer) {
    if (isdigit(lexer_peek(lexer))) {
        const char* begin = lexer->source.current;
        Location loc = lexer->source.loc;
        const char* end = lexer_skip_while(lexer, isdigit);

        if (isalpha(*end)) {
            fprintf(stderr, "ERROR: ./%lu:%lu Unexpected letter near number literal\n", lexer->source.loc.col, lexer->source.loc.row);
            return 1;
        }

        char* expected_end;
        const Token token = {
            .type = TT_NUMBER,
            .loc = loc,
            .as = {
                .number = strtoul(begin, &expected_end, 10)
            },
        };
        assert(expected_end == end);

        arrput(lexer->tokens, token);
        return true;
    // TODO: Shrink this down
    switch (lexer_peek(lexer)) {
            case '+': case '-': case '*': case '/': {
                Location loc = lexer->source.loc;
                char saved = lexer_peek(lexer);
                lexer_next(lexer);
                const Token token = {
                    .type = TT_OPERATOR,
                    .loc = loc,
                    .as = {
                        .operator = saved,
                    },
                };
                arrput(lexer->tokens, token);
                return true;
            }
        }
    } 

    return false;
}

void token_print(Token t) {
    switch (t.type) {
        case TT_NUMBER: {
            printf("%lu:%lu %lu\n", t.loc.col, t.loc.row, t.as.number);
        }
        case TT_OPERATOR: {
            printf("%lu:%lu %c\n", t.loc.col, t.loc.row, t.as.operator);
        }
        case TT_COUNT: {}
    }
}

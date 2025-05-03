#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>

#include "lexer.h"

#include "../extern/stb_ds.h"

bool Lexer_is_finished(const Lexer* lexer) {
    return *lexer->source.current == 0;
}

char Lexer_next(Lexer* lexer) {
    if (*lexer->source.current == '\n') {
        lexer->source.loc.col++;
        lexer->source.loc.row = 1;
    } else {
        lexer->source.loc.row++;
    }
    return *(lexer->source.current++);
}
char Lexer_peek(const Lexer* lexer) {
    return *(lexer->source.current);
}

const char* Lexer_skip_while(Lexer* lexer, int (*pred)(int)) {
    while (pred(Lexer_peek(lexer)) && !Lexer_is_finished(lexer)) Lexer_next(lexer);
    return lexer->source.current;
}

const char* Lexer_skip_ws(Lexer* lexer) {
    return Lexer_skip_while(lexer, isspace);
}

bool Lexer_parse_token(Lexer* lexer) {
    if (isdigit(Lexer_peek(lexer))) {
        const char* begin = lexer->source.current;
        const char* end = Lexer_skip_while(lexer, isdigit);

        if (isalpha(*end)) {
            fprintf(stderr, "ERROR: ./%lu:%lu Unexpected letter near number literal\n", lexer->source.loc.col, lexer->source.loc.row);
            return 1;
        }

        char* expected_end;
        const Token token = {
            .type = TT_NUMBER,
            .as = {
                .number = strtoul(begin, &expected_end, 10)
            },
        };
        assert(expected_end == end);

        arrput(lexer->tokens, token);
        return true;
    } else if (Lexer_peek(lexer) == '+') {
        Lexer_next(lexer);
        const Token token = {
            .type = TT_OPERATOR,
            .as = {
                .operator = '+',
            },
        };
        arrput(lexer->tokens, token);
        return true;
    }
    return false;
}

void Token_print(Token t) {
    switch (t.type) {
        case TT_NUMBER: {
            printf("%lu\n", t.as.number);
        }
        case TT_OPERATOR: {
            printf("%c\n", t.as.operator);
        }
    }
}

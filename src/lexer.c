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
        lexer->source.loc.row++;
        lexer->source.loc.col = 1;
    } else {
        lexer->source.loc.col++;
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

static int isidentchar(int c) {
    return isalpha(c) || c == '_' || isdigit(c);
}

static int isnewline(int c) {
    return c == '\n';
}

bool lexer_parse_token(Lexer* lexer) {
    if (lexer_peek(lexer) == '#') lexer_skip_while(lexer, isnewline);

    if (isdigit(lexer_peek(lexer))) {
        const char* begin = lexer->source.current;
        Location loc = lexer->source.loc;
        const char* end = lexer_skip_while(lexer, isdigit);

        if (isalpha(*end)) {
            lexer->error = (LexerError) {
                .message = "Unexpected letter near number literal",
                .loc = lexer->source.loc
            };
            return false;
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
    }
    if (isalpha(lexer_peek(lexer)) || lexer_peek(lexer) == '_') {
        const char* begin = lexer->source.current;
        Location loc = lexer->source.loc;
        const char* end = lexer_skip_while(lexer, isidentchar);
        ptrdiff_t len = end - begin;
        char* str = arena_alloc(lexer->arena, len + 1);
        strncpy(str, begin,
                     len);
        str[len] = 0;
        const Token token = {
            .type = TT_IDENT,
            .loc = loc,
            .as = {
                .ident = str
            },
        };
        arrput(lexer->tokens, token);
        return true;
    }

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
        case ';': {
            Location loc = lexer->source.loc;
            lexer_next(lexer);
            const Token token = {
                .type = TT_SEMICOLON,
                .loc = loc,
            };
            arrput(lexer->tokens, token);
            return true;
        }
        case ':': {
            Location loc = lexer->source.loc;
            lexer_next(lexer);
            const Token token = {
                .type = TT_COLON,
                .loc = loc,
            };
            arrput(lexer->tokens, token);
            return true;
        }
        case '=': {
            Location loc = lexer->source.loc;
            lexer_next(lexer);
            const Token token = {
                .type = TT_EQUAL,
                .loc = loc,
            };
            arrput(lexer->tokens, token);
            return true;
        }
        case '(': {
            Location loc = lexer->source.loc;
            lexer_next(lexer);
            const Token token = {
                .type = TT_OPENPAREN,
                .loc = loc,
            };
            arrput(lexer->tokens, token);
            return true;
        }
        case ')': {
            Location loc = lexer->source.loc;
            lexer_next(lexer);
            const Token token = {
                .type = TT_CLOSEPAREN,
                .loc = loc,
            };
            arrput(lexer->tokens, token);
            return true;
        }
    }

    lexer->error = (LexerError) {
        .message = "Unexpected character found",
        .loc = lexer->source.loc
    };
    return false;
}

void lexer_error_display(LexerError error, char* file_content, char* input_name) {
    long offset = get_offset_in_buffer(file_content, error.loc);
    char* line_start = file_content + offset - error.loc.col + 1;
    char* line_end = file_content + offset;
    while (*line_end && *line_end != '\n') line_end++;

    fprintf(stderr, "[lexer::error] %s:%lu:%lu: %s\n",
            input_name, error.loc.row, error.loc.col, error.message);

    fwrite(line_start, 1, line_end - line_start, stderr);
    fputc('\n', stderr);
    ptrdiff_t line_offset = error.loc.col - 1;
    fprintf(stderr, "%*s^\n", (int)line_offset, " ");
}


void token_print(Token t) {
    switch (t.type) {
        case TT_NUMBER: {
            printf("%lu:%lu %lu\n", t.loc.row, t.loc.col, t.as.number);
            break;
        }
        case TT_OPERATOR: {
            printf("%lu:%lu %c\n", t.loc.row, t.loc.col, t.as.operator);
            break;
        }
        case TT_SEMICOLON: {
            printf("%lu:%lu ;\n", t.loc.row, t.loc.col);
            break;
        }
        case TT_COLON: {
            printf("%lu:%lu :\n", t.loc.row, t.loc.col);
            break;
        }
        case TT_EQUAL: {
            printf("%lu:%lu =\n", t.loc.row, t.loc.col);
            break;
        }
        case TT_OPENPAREN: {
            printf("%lu:%lu (\n", t.loc.row, t.loc.col);
            break;
        }
        case TT_CLOSEPAREN: {
            printf("%lu:%lu )\n", t.loc.row, t.loc.col);
            break;
        }
        case TT_IDENT: {
            printf("%lu:%lu %s\n", t.loc.row, t.loc.col, t.as.ident);
            break;
        }
        case TT_COUNT: {}
    }
}

long get_offset_in_buffer(const char *buffer, Location loc) {
    int current_row = 1;
    long offset = 0;

    while (*buffer && current_row < loc.row) {
        if (*buffer == '\n') current_row++;
        buffer++;
        offset++;
    }

    if (current_row != loc.row) {
        return -1;
    }

    return offset + (loc.col - 1);
}

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>

#define STB_DS_IMPLEMENTATION
#include "../extern/stb_ds.h"

#include "lexer.h"
#include "parser.h"
#include "arena.h"

// Returns a null terminaed string of the file in `name`
// On any error (except malloc) prints error with perror and returns NULL
char* read_file(const char* name);
Lexer lex_file(char* content, Arena* arena);

const char* TokenTypeReadable[TT_COUNT] = {
    [TT_NUMBER] = "Number",
    [TT_OPERATOR] = "Operator",
};


int main() {
    char* file_content = read_file("test.nsl");

    Arena arena = arena_new(1024 * 10);

    Lexer lexer = lex_file(file_content, &arena);

    Parser parser = {
        .tokens = lexer.tokens,
        .pos = 0,
        .statements = NULL,
        .arena = &arena
    };

    while (!parser_is_finished(&parser)) {
        arrput(parser.statements, parser_statement(&parser));
    }

    free(file_content);
    arrfree(lexer.tokens);
    arrfree(parser.statements);
    arena_delete(&arena);

	return 0;
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

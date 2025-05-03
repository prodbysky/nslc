#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>

#define STB_DS_IMPLEMENTATION
#include "../extern/stb_ds.h"

#include "lexer.h"

// Returns a null terminaed string of the file in `name`
// On any error (except malloc) prints error with perror and returns NULL
char* read_file(const char* name);

Token* lex_file(char* content);

int main() {
    char* file_content = read_file("test.nsl");


    Token* tokens = lex_file(file_content);

    for (ptrdiff_t i = 0; i < arrlen(tokens); i++) Token_print(tokens[i]);

    free(file_content);
    arrfree(tokens);

	return 0;
}

Token* lex_file(char* content) {
    Lexer lexer = {
        .source = {
            .first = content,
            .current = content,
            .loc = {.col = 1, .row = 1},
        },
        .tokens = NULL, 
    };
    
    while (!Lexer_is_finished(&lexer)) {
        Lexer_skip_ws(&lexer);
        if (Lexer_is_finished(&lexer)) break;
        if (!Lexer_parse_token(&lexer)) return NULL;
    }
    return lexer.tokens;
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

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
#include "qbe.h"

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "../nob.h"

// Returns a null terminaed string of the file in `name`
// On any error (except malloc) prints error with perror and returns NULL
char* read_file(const char* name);
Lexer lex_file(char* content, Arena* arena);

const char* TokenTypeReadable[TT_COUNT] = {
    [TT_NUMBER] = "Number",
    [TT_OPERATOR] = "Operator",
};


void usage(const char* prog_name) {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "    %s <input.nsl> [OPTIONS]\n", prog_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "    -o <output> : specifies the output executable name\n");
}


int main(int argc, char** argv) {
    char* output_name = "a.out";
    char* input_name = NULL;

    for (int i = 1; i < argc; i++) {
        printf("%s\n", argv[i]);
        if (strcmp("-o", argv[i]) == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "ERROR: Output name not specified\n");
                usage(argv[0]);
                return 1;
            }
            output_name = argv[i + 1];
            i++;
        } else {
            input_name = argv[i];
        }
    }

    if (input_name == NULL) {
        fprintf(stderr, "ERROR: Input file name not specified\n");
        usage(argv[0]);
        return 1;
    }

    char* file_content = read_file(input_name);

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

    QBEModule mod = qbe_module_new();
    QBEFunction* main = qbe_module_create_function(&mod, "main", QVT_WORD);
    QBEBlock* entry = qbe_function_push_block(main, "entry");
    QBEInstruction return_value = {
        .type = QIT_RETURN,
        .ret = {
            .i = 0
        },
    };

    qbe_block_push_ins(entry, return_value);

    FILE* qbe_ir_file = fopen("main.ssa", "w");
    qbe_module_write(&mod, qbe_ir_file);
    fclose(qbe_ir_file);

    Cmd cmd = {0};
    cmd_append(&cmd, "qbe", "-o", "main.s", "main.ssa");
    if (!cmd_run_sync_and_reset(&cmd)) return 1;
    cmd_append(&cmd, "cc", "-o", output_name, "main.s");
    if (!cmd_run_sync_and_reset(&cmd)) return 1;
    cmd_append(&cmd, "rm", "main.s", "main.ssa");
    if (!cmd_run_sync_and_reset(&cmd)) return 1;
    free(cmd.items);

    free(file_content);
    arrfree(lexer.tokens);
    arrfree(parser.statements);
    arena_delete(&arena);
    qbe_module_destroy(&mod);

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

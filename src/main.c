#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

#define STB_DS_IMPLEMENTATION
#include "../extern/stb_ds.h"

#include "lexer.h"
#include "parser.h"
#include "arena.h"
#include "qbe.h"
#include "codegen.h"
#include "type_checker.h"

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "../nob.h"

// Returns a null terminaed string of the file in `name`
// On any error (except malloc) prints error with perror and returns NULL
char* read_file(const char* name);

const char* TokenTypeReadable[TT_COUNT] = {
    [TT_NUMBER] = "Number",
    [TT_OPERATOR] = "Operator",
};

typedef struct {
    char* input_name;
    char* output_name;
} Args;

Args parse_from_argv(int argc, char** argv);
Lexer lex_file(char* content, char* content_file_name, Arena* arena);
Parser parse_file(Token* tokens, Arena* arena, char* file_name);
bool write_and_compile_ir(Codegen* codegen, Statement* statements, char* out_name);

int main(int argc, char** argv) {
    Args args = parse_from_argv(argc, argv);
    if (args.input_name == NULL) return 1;

    char* file_content = read_file(args.input_name);
    Arena arena = arena_new(1024 * 10);

    Lexer lexer = lex_file(file_content, args.input_name, &arena);
    if (lexer.tokens == NULL) return 1;

    Parser parser = parse_file(lexer.tokens, &arena, args.input_name);
    if (parser.statements == NULL) {
        free(file_content);
        arrfree(lexer.tokens);
        arena_delete(&arena);
        return 1;
    } 
    TypeChecker checker = {
        .ast = parser.statements,
        .err = false,
        .vars = NULL,
    };

    if (type_check(&checker)) {
        fprintf(stderr, "Found type error :)\n");
        return 1;
    }

    QBEModule mod = qbe_module_new();
    QBEFunction* main = qbe_module_create_function(&mod, "main", QVT_WORD);
    QBEBlock* entry = qbe_function_push_block(main, "entry");

    Codegen codegen = {
        .mod = mod, 
        .main = main, 
        .entry = entry, 
        .temp_count = 0, 
        .variables = NULL,
    };

    if (!write_and_compile_ir(&codegen, parser.statements, args.output_name)) return 1;

    free(file_content);
    arrfree(lexer.tokens);
    arrfree(parser.statements);
    for (ptrdiff_t i = 0; i < arrlen(codegen.variables); i++) {
        free(codegen.variables[i].name);
        free(codegen.variables[i].ptr_name);
    }
    arrfree(codegen.variables);

    arena_delete(&arena);
    qbe_module_destroy(&mod);

	return 0;
}

bool write_and_compile_ir(Codegen* codegen, Statement* statements, char* out_name) {
    generate_code(codegen, statements);

    FILE* qbe_ir_file = fopen("main.ssa", "w");
    qbe_module_write(&codegen->mod, qbe_ir_file);
    fclose(qbe_ir_file);

    Cmd cmd = {0};
    cmd_append(&cmd, "qbe", "-o", "main.s", "main.ssa");
    if (!cmd_run_sync_and_reset(&cmd)) return false;
    cmd_append(&cmd, "cc", "-o", out_name, "main.s");
    if (!cmd_run_sync_and_reset(&cmd)) return false;
    cmd_append(&cmd, "rm", "main.s", "main.ssa");
    if (!cmd_run_sync_and_reset(&cmd)) return false;
    free(cmd.items);
    return true;
}

void usage(const char* prog_name) {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "    %s <input.nsl> [OPTIONS]\n", prog_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "    -o <output> : specifies the output executable name\n");
}


Args parse_from_argv(int argc, char** argv) {
    Args args = {0};
    args.output_name = "a.out";
    for (int i = 1; i < argc; i++) {
        if (strcmp("-o", argv[i]) == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "ERROR: Output name not specified\n");
                usage(argv[0]);
                return (Args){0};
            }
            args.output_name = argv[i + 1];
            i++;
        } else {
            args.input_name = argv[i];
        }
    }

    if (args.input_name == NULL) {
        fprintf(stderr, "ERROR: Input file name not specified\n");
        usage(argv[0]);
        return (Args){0};
    }
    return args;
}

Lexer lex_file(char* content, char* content_file_name, Arena* arena) {
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
            lexer_error_display(lexer.error, content, content_file_name);
            arrfree(lexer.tokens);
            return (Lexer) {};
        }
    }
    return lexer;
}

Parser parse_file(Token* tokens, Arena* arena, char* file_name) {
    Parser parser = {
        .token_origin = file_name,
        .tokens = tokens,
        .pos = 0,
        .statements = NULL,
        .arena = arena
    };

    while (!parser_is_finished(&parser)) {
        if (!parser_statement(&parser, &parser.statements)) {
            fprintf(stderr, "Failed to parse file due to invalid statement\n");
            arrfree(parser.statements);
            parser.statements = NULL;
            return parser;
        }
    }
    return parser;
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

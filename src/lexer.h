#ifndef LEXER_H_
#define LEXER_H_
#include <stdbool.h>
#include <stdint.h>
#include "arena.h"

// Rust has been permanentely printed
// into my brain stem
typedef enum {
    TT_NUMBER,
    TT_OPERATOR,
    TT_IDENT,
    TT_SEMICOLON,
    TT_COUNT,
} TokenType;

// One indexed location
typedef struct {
    ptrdiff_t row, col;
} Location;

typedef struct {
    TokenType type;
    Location loc;
    union {
        uint64_t number;
        char operator;
        char* ident;
    } as;
} Token;


typedef struct {
    struct {
        char* first;
        char* current;
        Location loc;
    } source;
    Token* tokens;
    Arena* arena;
} Lexer;


bool lexer_is_finished(const Lexer* lexer);
char lexer_next(Lexer* lexer);
char lexer_peek(const Lexer* lexer);

const char* lexer_skip_while(Lexer* lexer, int (*pred)(int));
const char* lexer_skip_ws(Lexer* lexer);

bool lexer_parse_token(Lexer* lexer);
void token_print(Token t);

#endif

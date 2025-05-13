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
    TT_KEYWORD,
    TT_SEMICOLON,
    TT_COLON,
    TT_EQUAL,
    TT_OPENPAREN,
    TT_CLOSEPAREN,
    TT_OPENCURLY,
    TT_CLOSECURLY,
    TT_COUNT,
} TokenType;

typedef enum {
    TK_RETURN,
    TK_LET,
    TK_IF,
    TK_WHILE,
    TK_FALSE,
    TK_TRUE,
} TokenKeyword;

// One indexed location
typedef struct {
    ptrdiff_t col, row; // y:x
} Location;

typedef struct {
    TokenType type;
    Location loc;
    union {
        uint64_t number;
        char operator;
        char* ident;
        TokenKeyword keyword;
    } as;
} Token;

typedef struct {
    const char* message;
    Location loc;
} LexerError;

typedef struct {
    struct {
        char* first;
        char* current;
        Location loc;
    } source;
    Token* tokens;
    // This arena should live for the entirety of the int main() lifetime
    // Rust begin embroidered into my brain stem AGAIN
    Arena* arena;
    LexerError error;
} Lexer;


bool lexer_is_finished(const Lexer* lexer);
char lexer_next(Lexer* lexer);
char lexer_peek(const Lexer* lexer);

const char* lexer_skip_while(Lexer* lexer, int (*pred)(int));
const char* lexer_skip_ws(Lexer* lexer);

bool lexer_parse_token(Lexer* lexer);
void token_print(Token t);
void lexer_error_display(LexerError error, char* file_content, char* input_name);
long get_offset_in_buffer(const char *buffer, Location loc);

#endif

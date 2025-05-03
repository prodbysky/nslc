#ifndef LEXER_H_
#define LEXER_H_
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    TT_NUMBER,
    TT_OPERATOR,
} TokenType;

typedef struct {
    TokenType type;
    union {
        uint64_t number;
        char operator;
    } as;
} Token;

typedef struct {
    ptrdiff_t row, col;
} Location;

typedef struct {
    struct {
        char* first;
        char* current;
        Location loc;
    } source;
    Token* tokens;
} Lexer;


bool Lexer_is_finished(const Lexer* lexer);
char Lexer_next(Lexer* lexer);
char Lexer_peek(const Lexer* lexer);

const char* Lexer_skip_while(Lexer* lexer, int (*pred)(int));
const char* Lexer_skip_ws(Lexer* lexer);

bool Lexer_parse_token(Lexer* lexer);
void Token_print(Token t);

#endif

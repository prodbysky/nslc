#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include "arena.h"
#include "parser.h"

typedef enum {
    CT_INT,
    CT_BOOL,
    CT_ERROR,
} CheckerType;

typedef struct {
    char* name;
    CheckerType type;
} CheckerVariable;

typedef struct {
    const Statement* ast;
    CheckerVariable* vars;
    bool err;
} TypeChecker;

bool type_check(TypeChecker* checker);
#endif

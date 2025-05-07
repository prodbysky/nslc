#ifndef CODEGEN_H 
#define CODEGEN_H

#include "qbe.h"
#include <stddef.h>
#include "parser.h"

typedef struct {
    char* name;
    char* ptr_name;
} Variable;

typedef struct {
    QBEModule mod;
    QBEFunction* main;
    QBEBlock* entry;
    Variable* variables;
    size_t temp_count;
} Codegen;



void generate_code(Codegen* codegen, Statement* sts);
QBEValue generate_expr(Codegen* codegen, const Expr* expr);
char* fresh_temp(Codegen* codegen);

#endif

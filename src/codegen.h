#ifndef CODEGEN_H
#define CODEGEN_H

#include "qbe.h"
#include <stddef.h>
#include "parser.h"

typedef struct {
    QBEModule mod;
    QBEFunction* main;
    QBEBlock* entry;
    size_t temp_count;
} Codegen;

void generate_code(Codegen* codegen, Statement* sts);
QBEValue generate_expr(Codegen* codegen, const Expr* expr);
char* fresh_temp(Codegen* codegen);

#endif

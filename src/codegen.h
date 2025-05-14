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
void generate_statement(Codegen* codegen, Statement st, QBEBlock* block);
QBEValue generate_expr(Codegen* codegen, const Expr* expr, QBEBlock* block);
char* fresh_temp(Codegen* codegen);

void generate_store(QBEBlock* block, QBEValue val, char* into);
QBEValue generate_cmp(QBEBlock* block, QBEComparisonType cmp, QBEValueType element_type, QBEValue l, QBEValue r, QBEValue into);
#endif

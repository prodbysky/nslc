#ifndef QBE_H
#define QBE_H
#include "arena.h"
#include <stdint.h>
#include <stdio.h>


typedef enum {
    QIT_RETURN
} QBEInstructionType;

typedef enum {
    QVT_WORD
} QBEValueType;

typedef struct {
    uint64_t i;
} QBEValue;

typedef struct {
    QBEInstructionType type;
    union {
        QBEValue ret;
    };
} QBEInstruction;

typedef struct {
    char* name;
    QBEInstruction* instructions;
} QBEBlock;

typedef struct {
    char* name;
    QBEValueType return_type;
    QBEBlock* blocks;
} QBEFunction;

typedef struct {
    QBEFunction* functions;
} QBEModule;

QBEModule qbe_module_new();
void qbe_module_destroy(QBEModule* module);
QBEFunction* qbe_module_create_function(QBEModule* module, char* name, QBEValueType return_type);

QBEBlock* qbe_function_push_block(QBEFunction* function, char* name);
void qbe_block_push_ins(QBEBlock* block, QBEInstruction ins);

void qbe_module_write(const QBEModule* module, FILE* file);
void qbe_function_write(const QBEFunction* function, FILE* file);
void qbe_block_write(const QBEBlock* block, FILE* file);
void qbe_instruction_write(const QBEInstruction* instruction, FILE* file);

#endif

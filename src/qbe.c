#include "qbe.h"

#include "../extern/stb_ds.h"
#include <assert.h>


QBEModule qbe_module_new() {
    return (QBEModule) {
        .functions = NULL
    };
}

void qbe_module_destroy(QBEModule* module) {
    for (ptrdiff_t f = 0; f < arrlen(module->functions); f++) {
        QBEFunction* func = &module->functions[f];
        for (ptrdiff_t b = 0; b < arrlen(func->blocks); b++) {
            QBEBlock* block = &func->blocks[b];
            arrfree(block->instructions);
        }
        arrfree(func->blocks);
    }
    arrfree(module->functions);
}

QBEFunction* qbe_module_create_function(QBEModule* module, char* name, QBEValueType return_type) {
    ptrdiff_t loc = arrlen(module->functions);
    QBEFunction func = {
        .name = name,
        .return_type = return_type,
        .blocks = NULL
    };
    arrput(module->functions, func);
    return &module->functions[loc];
}

QBEBlock* qbe_function_push_block(QBEFunction* function, char* name) {
    ptrdiff_t loc = arrlen(function->blocks);
    QBEBlock block = {
        .name = name,
        .instructions = NULL
    };
    arrput(function->blocks, block);
    return &function->blocks[loc];
}

void qbe_block_push_ins(QBEBlock* block, QBEInstruction ins) {
    arrput(block->instructions, ins);
}

void qbe_module_write(const QBEModule* module, FILE* file) {
    for (ptrdiff_t i = 0; i < arrlen(module->functions); i++) {
        qbe_function_write(&module->functions[i], file);
    }
}
void qbe_function_write(const QBEFunction* function, FILE* file) {
    fprintf(file, "export function w $%s() {\n", function->name);
    for (ptrdiff_t i = 0; i < arrlen(function->blocks); i++) {
        qbe_block_write(&function->blocks[i], file);
    }
    fprintf(file, "}\n");
}

void qbe_block_write(const QBEBlock* block, FILE* file) {
    fprintf(file, "@%s\n", block->name);
    for (ptrdiff_t i = 0; i < arrlen(block->instructions); i++) {
        qbe_instruction_write(&block->instructions[i], file);
    }
}
void qbe_instruction_write(const QBEInstruction* instruction, FILE* file) {
    switch (instruction->type) {
        case QIT_RETURN: {
            fprintf(file, "ret %ld\n", instruction->ret.i);
        }
    }
}

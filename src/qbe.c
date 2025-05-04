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


            for (ptrdiff_t s = 0; s < arrlen(block->statements); s++) {
                QBEStatement* st = &block->statements[s];
                switch (st->type) {
                    case QST_ASSIGN: {
                        free(st->assign.value.name);
                        break;
                    }
                    default: {}
                }
            }
            arrfree(block->statements);
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
        .statements = NULL
    };
    arrput(function->blocks, block);
    return &function->blocks[loc];
}

void qbe_block_push_ins(QBEBlock* block, QBEInstruction ins) {
    const QBEStatement st = {
        .type = QST_THROWAWAY,
        .throwaway = ins
    };
    arrput(block->statements, st);
}

void qbe_block_assign_ins(QBEBlock* block, QBEInstruction ins, QBEValueType type, QBEValue val) {
    const QBEStatement st = {
        .type = QST_ASSIGN,
        .assign = {
            .value = val,
            .type = type,
            .instruction = ins
        }
    };
    arrput(block->statements, st);
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
    for (ptrdiff_t i = 0; i < arrlen(block->statements); i++) {
        qbe_statement_write(&block->statements[i], file);
    }
}
void qbe_statement_write(const QBEStatement* statement, FILE* file) {

    if (statement->type == QST_ASSIGN) {
        assert(statement->assign.value.kind == QVK_TEMP);
        fprintf(file, "%%%s =", statement->assign.value.name);
        switch (statement->assign.type) {
            case QVT_WORD: {
                fprintf(file, "w ");
                break;
            }
        }
        qbe_instruction_write(&statement->assign.instruction, file);
    } else {
        qbe_instruction_write(&statement->throwaway, file);
    }
}

void qbe_instruction_write(const QBEInstruction* instruction, FILE* file) {
    switch (instruction->type) {
        case QIT_RETURN: {
            fprintf(file, "ret ");
            switch (instruction->ret.kind) {
                case QVK_CONST: fprintf(file, "%lu", instruction->ret.const_i); break;
                case QVK_TEMP: fprintf(file, "%%%s", instruction->ret.name); break;
            }
            fprintf(file, "\n");
            break;
        }
        case QIT_ADD: {
            fprintf(file, "add ");
            switch (instruction->add.left.kind) {
                case QVK_CONST: fprintf(file, "%lu, ", instruction->add.left.const_i); break;
                case QVK_TEMP: fprintf(file, "%%%s, ", instruction->add.left.name); break;
            }
            switch (instruction->add.right.kind) {
                case QVK_CONST: fprintf(file, "%lu", instruction->add.right.const_i); break;
                case QVK_TEMP: fprintf(file, "%%%s", instruction->add.right.name); break;
            }
            fprintf(file, "\n");
            break;
        }
        case QIT_SUB: {
            fprintf(file, "sub ");
            switch (instruction->sub.left.kind) {
                case QVK_CONST: fprintf(file, "%lu, ", instruction->sub.left.const_i); break;
                case QVK_TEMP: fprintf(file, "%%%s, ", instruction->sub.left.name); break;
            }
            switch (instruction->sub.right.kind) {
                case QVK_CONST: fprintf(file, "%lu", instruction->sub.right.const_i); break;
                case QVK_TEMP: fprintf(file, "%%%s", instruction->sub.right.name); break;
            }
            fprintf(file, "\n");
            break;
        }
        case QIT_MUL: {
            fprintf(file, "mul ");
            switch (instruction->mul.left.kind) {
                case QVK_CONST: fprintf(file, "%lu, ", instruction->mul.left.const_i); break;
                case QVK_TEMP: fprintf(file, "%%%s, ", instruction->mul.left.name); break;
            }
            switch (instruction->mul.right.kind) {
                case QVK_CONST: fprintf(file, "%lu", instruction->mul.right.const_i); break;
                case QVK_TEMP: fprintf(file, "%%%s", instruction->mul.right.name); break;
            }
            fprintf(file, "\n");
            break;
        }
        case QIT_DIV: {
            fprintf(file, "div ");
            switch (instruction->div.left.kind) {
                case QVK_CONST: fprintf(file, "%lu, ", instruction->div.left.const_i); break;
                case QVK_TEMP: fprintf(file, "%%%s, ", instruction->div.left.name); break;
            }
            switch (instruction->div.right.kind) {
                case QVK_CONST: fprintf(file, "%lu", instruction->div.right.const_i); break;
                case QVK_TEMP: fprintf(file, "%%%s", instruction->div.right.name); break;
            }
            fprintf(file, "\n");
            break;
        }
    } 
}

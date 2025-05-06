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

void qbe_write_left_right(const QBEValue* left, const QBEValue* right, FILE* file) {
    switch (left->kind) {
        case QVK_CONST: fprintf(file, "%lu, ", left->const_i); break;
        case QVK_TEMP: fprintf(file, "%%%s, ", left->name); break;
    }
    switch (right->kind) {
        case QVK_CONST: fprintf(file, "%lu", right->const_i); break;
        case QVK_TEMP: fprintf(file, "%%%s", right->name); break;
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
            qbe_write_left_right(&instruction->add.left, &instruction->add.right, file);
            fprintf(file, "\n");
            break;
        }
        case QIT_SUB: {
            fprintf(file, "sub ");
            qbe_write_left_right(&instruction->sub.left, &instruction->sub.right, file);
            fprintf(file, "\n");
            break;
        }
        case QIT_MUL: {
            fprintf(file, "mul ");
            qbe_write_left_right(&instruction->mul.left, &instruction->mul.right, file);
            fprintf(file, "\n");
            break;
        }
        case QIT_DIV: {
            fprintf(file, "div ");
            qbe_write_left_right(&instruction->div.left, &instruction->div.right, file);
            fprintf(file, "\n");
            break;
        }
        case QIT_ALLOC4: {
            fprintf(file, "alloc4 ");
            fprintf(file, "%lu", instruction->alloc4.size);
            fprintf(file, "\n");
            break;
        }
        case QIT_STOREW: {
            fprintf(file, "storew ");
            switch (instruction->storew.value.kind) {
                case QVK_CONST: fprintf(file, "%lu, ", instruction->storew.value.const_i); break;
                case QVK_TEMP: fprintf(file, "%%%s, ", instruction->storew.value.name); break;
            }
            fprintf(file, "%%%s", instruction->storew.name);
            fprintf(file, "\n");
            break;
        }
        case QIT_LOADW: {
            fprintf(file, "loadw ");
            fprintf(file, "%%%s", instruction->loadw.name);
            fprintf(file, "\n");
            break;
        }
    } 
}

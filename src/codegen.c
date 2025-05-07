#include "codegen.h"
#include <assert.h>
#include <string.h>
#include "../extern/stb_ds.h"

char* fresh_temp(Codegen* codegen) {
    char buffer[32];
    memset(buffer, 0, 32);
    const size_t storage_index = codegen->temp_count++;
    snprintf(buffer, 32, "t%zu", storage_index);
    return strdup(buffer);
}

void generate_code(Codegen* codegen, Statement* sts) {
    for (ptrdiff_t i = 0; i < arrlen(sts); i++) {
        Statement* st = &sts[i];
        switch (st->type) {
            case ST_RETURN: {
                QBEValue value = generate_expr(codegen, st->as.ret);
                qbe_block_push_ins(codegen->entry, (QBEInstruction) {
                    .type = QIT_RETURN,
                    .ret = value
                });
                break;
            }
            case ST_VARIABLE_DEFINE: {
                char* var_temp = fresh_temp(codegen);
                QBEValue alloc_result = {.kind = QVK_TEMP, .name = var_temp };
                
                qbe_block_assign_ins(
                    codegen->entry, 
                    (QBEInstruction) {
                        .type = QIT_ALLOC4,
                        .alloc4.size = 4
                    }, 
                    QVT_WORD, 
                    alloc_result
                );
                
                hmput(codegen->variables, st->as.var_def.name, var_temp);

                for (ptrdiff_t a = 0; a < hmlen(codegen->variables); a++) {
                    printf("key: %s, value: %s\n", codegen->variables[a].key, codegen->variables[a].value);
                }
                
                QBEValue value = generate_expr(codegen, st->as.var_def.value);
                
                qbe_block_push_ins(
                    codegen->entry, 
                    (QBEInstruction) {
                        .type = QIT_STOREW,
                        .storew = {
                            .value = value,
                            .name = var_temp
                        }
                    }
                );
                break;
            }
            case ST_ERROR: {
                fprintf(stderr, "Found some invalid statement during parsing\n");
                return;
            } 
        }
    }
}

QBEValue generate_expr(Codegen* codegen, const Expr* expr) {
    switch (expr->type) {
        case ET_NUMBER: {
            return (QBEValue) {
                .kind = QVK_CONST,
                .const_i = expr->as.number
            };
        }
        case ET_VARIABLE: {
            char* var_loc = hmget(codegen->variables, expr->as.variable);
            printf("%s\n", expr->as.variable);

            // Create a temporary for the loaded value
            char* place = fresh_temp(codegen);
            QBEValue result = {.kind = QVK_TEMP, .name = place };
            
            qbe_block_assign_ins(
                codegen->entry, 
                (QBEInstruction) {
                    .type = QIT_LOADW,
                    .loadw.name = var_loc
                }, 
                QVT_WORD, 
                result
            );
            return result;
        }
        case ET_BINARY: {
            QBEValue left = generate_expr(codegen, expr->as.binary.left);
            QBEValue right = generate_expr(codegen, expr->as.binary.right);
            switch (expr->as.binary.op) {
                case '+': {
                    char* name = fresh_temp(codegen);
                    QBEValue result = { .kind = QVK_TEMP, .name = name };

                    qbe_block_assign_ins(
                        codegen->entry, 
                        (QBEInstruction) {
                            .type = QIT_ADD,
                            .add = { .left = left, .right = right }, 
                        }, 
                        QVT_WORD, 
                        result
                    );
                    return result; 
                }
                case '-': {
                    char* name = fresh_temp(codegen);
                    QBEValue result = { .kind = QVK_TEMP, .name = name };

                    qbe_block_assign_ins(
                        codegen->entry, 
                        (QBEInstruction) {
                            .type = QIT_SUB,
                            .sub = { .left = left, .right = right }, 
                        }, 
                        QVT_WORD, 
                        result
                    );
                    return result; 
                }
                case '*': {
                    char* name = fresh_temp(codegen);

                    QBEValue result = { .kind = QVK_TEMP, .name = name };

                    qbe_block_assign_ins(
                        codegen->entry, 
                        (QBEInstruction) {
                            .type = QIT_MUL,
                            .mul = { .left = left, .right = right }, 
                        }, 
                        QVT_WORD, 
                        result
                    );
                    return result; 
                }
                case '/': {
                    char* name = fresh_temp(codegen);

                    QBEValue result = { .kind = QVK_TEMP, .name = name };

                    qbe_block_assign_ins(
                        codegen->entry, 
                        (QBEInstruction) {
                            .type = QIT_DIV,
                            .div = { .left = left, .right = right }, 
                        }, 
                        QVT_WORD, 
                        result
                    );
                    return result; 
                }
            }
        }
    }
    assert(false && "Not implemented");
}

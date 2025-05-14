#include "codegen.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "../extern/stb_ds.h"
#include "parser.h"
#include "qbe.h"

char* fresh_temp(Codegen* codegen) {
    char buffer[32];
    memset(buffer, 0, 32);
    const size_t storage_index = codegen->temp_count++;
    snprintf(buffer, 32, "t%zu", storage_index);
    return strdup(buffer);
}

char* fresh_label(Codegen* codegen, char* prefix) {
    char buffer[32];
    memset(buffer, 0, 32);
    const size_t storage_index = codegen->temp_count++;
    snprintf(buffer, 32, "%s%zu", prefix, storage_index);
    return strdup(buffer);
}

void generate_code(Codegen* codegen, Statement* sts) {
    for (ptrdiff_t i = 0; i < arrlen(sts); i++) {
        generate_statement(codegen, sts[i], codegen->entry);
    }
}

QBEValue generate_expr(Codegen* codegen, const Expr* expr, QBEBlock* block) {
    switch (expr->type) {
        case ET_NUMBER: {
            return (QBEValue) {
                .kind = QVK_CONST,
                .const_i = expr->as.number
            };
        }
        case ET_BOOL: {
            return (QBEValue) {
                .kind = QVK_CONST,
                .const_i = expr->as.boolean
            };
        }
        case ET_VARIABLE: {
            Variable v = {0};
            for (ptrdiff_t i = 0; i < arrlen(codegen->variables); i++) {
                if (strcmp(codegen->variables[i].name, expr->as.variable) == 0) {
                    v = codegen->variables[i];
                    break;
                }
            }
            char* place = fresh_temp(codegen);
            QBEValue result = {.kind = QVK_TEMP, .name = place };
            
            qbe_block_assign_ins(
                codegen->entry, 
                (QBEInstruction) {
                    .type = QIT_LOADW,
                    .loadw.name = v.ptr_name
                }, 
                QVT_WORD, 
                result
            );
            return result;
        }
        case ET_BINARY: {
            QBEValue left = generate_expr(codegen, expr->as.binary.left, block);
            QBEValue right = generate_expr(codegen, expr->as.binary.right, block);
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
                case '>': {
                    char* name = fresh_temp(codegen);
                    QBEValue result = { .kind = QVK_TEMP, .name = name };
                    return generate_cmp(block, QCT_GT, QVT_WORD, left, right, result);
                }
                case '<': {
                    char* name = fresh_temp(codegen);
                    QBEValue result = { .kind = QVK_TEMP, .name = name };
                    return generate_cmp(block, QCT_LT, QVT_WORD, left, right, result);
                }
            }
        }
    }
    assert(false && "Not implemented");
}

void generate_statement(Codegen* codegen, Statement st, QBEBlock* block) {
        switch (st.type) {
            case ST_SET_VARIABLE: {
                QBEValue new_value = generate_expr(codegen, st.as.var_assign.new_val, block);

                Variable v = {0};
                for (ptrdiff_t i = 0; i < arrlen(codegen->variables); i++) {
                    if (strcmp(codegen->variables[i].name, st.as.var_assign.var) == 0) {
                        v = codegen->variables[i];
                        break;
                    }
                }
                generate_store(block, new_value, v.ptr_name);
                return;
            }
            case ST_IF: {
                QBEValue cond = generate_expr(codegen, st.as.if_st.cond, block);
                char* then_label_name = fresh_label(codegen, "then_");
                char* else_label_name = fresh_label(codegen, "else_");
                char* cond_name = fresh_temp(codegen);
                QBEValue cond_place = { .name = cond_name, .kind = QVK_TEMP };
                const QBEValue zero = { .kind = QVK_CONST, .const_i = 0 };
                generate_cmp(block, QCT_NE, QVT_WORD, cond, zero, cond_place);
                qbe_block_push_ins(codegen->entry, (QBEInstruction) {
                    .type = QIT_JNZ,
                    .jnz = {.then = then_label_name, .otherwise = else_label_name, .value = cond_place}
                });
                qbe_block_push_label(codegen->entry, then_label_name);
                for (ptrdiff_t i = 0; i < arrlen(st.as.if_st.body); i++) generate_statement(codegen, st.as.if_st.body[i], block);
                qbe_block_push_label(codegen->entry, else_label_name);
                return;
            }
            case ST_RETURN: {
                QBEValue value = generate_expr(codegen, st.as.ret, block);
                qbe_block_push_ins(codegen->entry, (QBEInstruction) {
                    .type = QIT_RETURN,
                    .ret = value
                });
                return;
            }
            case ST_VARIABLE_DEFINE: {
                char* var_temp = fresh_temp(codegen);
                QBEValue alloc_result = {.kind = QVK_TEMP, .name = var_temp };
                
                qbe_block_assign_ins(
                    codegen->entry, 
                    (QBEInstruction) {
                        .type = QIT_ALLOC8,
                        .alloc8.size = 1
                    }, 
                    QVT_LONG, 
                    alloc_result
                );
                
                Variable v = {
                    .name = strdup(st.as.var_def.name),
                    .ptr_name = strdup(var_temp),
                };
                arrput(codegen->variables, v);
                
                QBEValue value = generate_expr(codegen, st.as.var_def.value, block);
                
                generate_store(block, value, var_temp);
                return;
            }
            case ST_WHILE: {
                char* header_label_name = fresh_label(codegen, "header_");
                char* body_label_name = fresh_label(codegen, "body_");
                char* out_label_name = fresh_label(codegen, "out_");
                char* cond_name = fresh_temp(codegen);
                qbe_block_push_label(codegen->entry, header_label_name);
                QBEValue cond = generate_expr(codegen, st.as.while_st.cond, block);
                QBEValue cond_place = {
                    .name = cond_name,
                    .kind = QVK_TEMP,
                };
                const QBEValue zero = {
                    .kind = QVK_CONST,
                    .const_i = 0
                };
                generate_cmp(block, QCT_NE, QVT_WORD, cond, zero, cond_place);
                qbe_block_push_ins(codegen->entry, (QBEInstruction) {
                    .type = QIT_JNZ,
                    .jnz = {.then = body_label_name, .otherwise = out_label_name, .value = cond_place}
                });
                qbe_block_push_label(codegen->entry, body_label_name);
                for (ptrdiff_t i = 0; i < arrlen(st.as.if_st.body); i++) {
                    generate_statement(codegen, st.as.if_st.body[i], block);
                }
                qbe_block_push_ins(codegen->entry, (QBEInstruction) { .type = QIT_JMP, .jmp = { .label = header_label_name } });
                qbe_block_push_label(codegen->entry, out_label_name);
                return;
            }
            case ST_ERROR: {
                fprintf(stderr, "Found some invalid statement during parsing\n");
                return;
            } 
        }
}

void generate_store(QBEBlock* block, QBEValue val, char* into) {
    qbe_block_push_ins(block, (QBEInstruction) {
        .type = QIT_STOREW,
        .storew = {
            .value = val,
            .name = into,
        }
    });
}
QBEValue generate_cmp(QBEBlock* block, QBEComparisonType cmp, QBEValueType element_type, QBEValue l, QBEValue r, QBEValue into) { 
    qbe_block_assign_ins(block, (QBEInstruction) {
        .type = QIT_CMP,
        .cmp = {
            .type = element_type,
            .cmp = cmp,
            .l = l,
            .r = r
        }
    }, QVT_WORD, into);
    return into;
}

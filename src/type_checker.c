#include "type_checker.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "../extern/stb_ds.h"
#include "parser.h"


static void type_check_st(TypeChecker* checker, Statement st);
static CheckerType type_check_expr(TypeChecker* checker, Expr* expr);

bool type_check(TypeChecker* checker) {
    for (ptrdiff_t i = 0; i < arrlen(checker->ast); i++) type_check_st(checker, checker->ast[i]);
    return checker->err;
}

CheckerVariable fn_arg_to_checker_var(FnArg arg) {
    if (strcmp(arg.type, "i32") == 0) {
        return (CheckerVariable) {
            .type = CT_INT,
            .name = arg.name
        };
    }
    if (strcmp(arg.type, "bool") == 0) {
        return (CheckerVariable) {
            .type = CT_BOOL,
            .name = arg.name
        };
    }
    assert(false);
}

static void type_check_st(TypeChecker* checker, Statement st) {
    switch (st.type) {
        case ST_FN_DEFINITION: {
            CheckerVariable* saved = checker->vars;
            checker->vars = NULL;
            for (ptrdiff_t i = 0; i < arrlen(st.as.fn_def.args); i++) {
                arrput(checker->vars, fn_arg_to_checker_var(st.as.fn_def.args[i]));
            }
            for (ptrdiff_t i = 0; i < arrlen(st.as.fn_def.body); i++) {
                type_check_st(checker, st.as.fn_def.body[i]);
            }

            arrfree(checker->vars);
            checker->vars = saved;
            return;
        }
        case ST_VARIABLE_DEFINE: {
            CheckerType expression_type = type_check_expr(checker, st.as.var_def.value);
            if (expression_type == CT_ERROR) {
                checker->err = true;
                return;
            }
            switch (expression_type) {
                case CT_INT: {
                    if (strcmp("i32", st.as.var_def.type) != 0) {
                        checker->err = true;
                        return;
                    }
                    break;
                }
                case CT_BOOL: {
                    if (strcmp("bool", st.as.var_def.type) != 0) {
                        checker->err = true;
                        return;
                    }
                    break;
                }
                case CT_ERROR: {
                    assert(false && "Unreachable");
                }
            }
            CheckerVariable v = {
                .type = expression_type,
                .name = st.as.var_def.name
            };
            arrput(checker->vars, v);

            break;
        }
        case ST_WHILE: {
            CheckerType expression_type = type_check_expr(checker, st.as.while_st.cond);
            if (expression_type != CT_BOOL) {
                checker->err = true;
                return;
            }
            for (ptrdiff_t i = 0; i < arrlen(st.as.while_st.body); i++) {
                type_check_st(checker, st.as.while_st.body[i]);
            }
            return;
        }
        case ST_IF: {
            CheckerType expression_type = type_check_expr(checker, st.as.if_st.cond);
            if (expression_type != CT_BOOL) {
                checker->err = true;
                return;
            }
            for (ptrdiff_t i = 0; i < arrlen(st.as.if_st.body); i++) {
                type_check_st(checker, st.as.if_st.body[i]);
            }
            return;
        }
        case ST_SET_VARIABLE: {
            CheckerVariable v = {0};
            for (ptrdiff_t i = 0; i < arrlen(checker->vars); i++) {
                if (strcmp(checker->vars[i].name, st.as.var_assign.var) == 0) v = checker->vars[i];
            }
            if (v.name == NULL) {
                checker->err = true;
                return;
            }
            CheckerType expression_type = type_check_expr(checker, st.as.var_assign.new_val);
            if (v.type != expression_type) {
                checker->err = true;
                return;
            }
            return;
        }
        case ST_ERROR: case ST_RETURN: {
            return;
        }
    }
}

static CheckerType type_check_expr(TypeChecker* checker, Expr* expr) {
    switch (expr->type) {
        case ET_NUMBER: {
            return CT_INT;
        }
        case ET_BOOL: {
            return CT_BOOL;
        }
        case ET_BINARY: {
            CheckerType left = type_check_expr(checker, expr->as.binary.left); if (left == CT_ERROR) return CT_ERROR;
            CheckerType right = type_check_expr(checker, expr->as.binary.right); if (right == CT_ERROR) return CT_ERROR;
            if (left != right) {
                return CT_ERROR;
            }
            switch (expr->as.binary.op) {
                case '+': case '-': case '*': case '/': {
                    return CT_INT;
                }
                case '>': case '<': {
                    return CT_BOOL;
                }
            }
            return CT_ERROR;
        }
        case ET_VARIABLE: {
            for (ptrdiff_t i = 0; i < arrlen(checker->vars); i++) {
                if (strcmp(checker->vars[i].name, expr->as.variable) == 0) return checker->vars[i].type;
            }
            return CT_ERROR;
        }
    }
    return CT_ERROR;
}

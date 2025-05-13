#ifndef QBE_H
#define QBE_H
#include "arena.h"
#include <stdint.h>
#include <stdio.h>


typedef enum {
    QIT_RETURN,
    QIT_ADD,
    QIT_SUB,
    QIT_MUL,
    QIT_DIV,
    QIT_ALLOC8,
    QIT_STOREW,
    QIT_LOADW,
    QIT_CMP,
    QIT_JMP,
    QIT_JNZ,
} QBEInstructionType;

typedef enum {
    QCT_NE,
    QCT_GT,
    QCT_LT,
} QBEComparisonType;

typedef enum {
    QVT_WORD,
    QVT_LONG,
} QBEValueType;

typedef enum {
    QVK_CONST, // constant values: numbers ...
    QVK_TEMP // %value
} QBEValueKind;

typedef struct {
    QBEValueKind kind;
    union {
        uint64_t const_i;
        char* name;
    };
} QBEValue;

typedef struct {
    QBEInstructionType type;
    union {
        QBEValue ret;
        struct {
            QBEValue left;
            QBEValue right;
        } add, sub, mul, div;
        struct {
            size_t size;
        } alloc8;
        struct {
            QBEValue value;
            char* name;
        } storew;
        struct {
            char* name;
        } loadw;
        struct {
            char* label;
        } jmp;
        struct {
            char* then;
            char* otherwise;
            QBEValue value;
        } jnz;
        struct {
            QBEValueType type;
            QBEComparisonType cmp;
            QBEValue l, r;
        } cmp;
    };
} QBEInstruction;

typedef enum {
    QST_ASSIGN, // %value w= instr
    QST_THROWAWAY, // instr
	QST_LABEL // @label
} QBEStatementType;

typedef struct {
    QBEStatementType type;
    union {
        QBEInstruction throwaway;
        struct {
            QBEValue value;
            QBEValueType type;
            QBEInstruction instruction;
        } assign;
		char* name;
    };
} QBEStatement;

typedef struct {
    char* name;
    QBEStatement* statements;
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
// Pushes instruction throwing away its return value
void qbe_block_push_ins(QBEBlock* block, QBEInstruction ins);
// Pushes instruction that stores its return value into val (has to be a temporary value)
void qbe_block_assign_ins(QBEBlock* block, QBEInstruction ins, QBEValueType type, QBEValue val);

void qbe_block_push_label(QBEBlock* block, char* name);

void qbe_module_write(const QBEModule* module, FILE* file);
void qbe_function_write(const QBEFunction* function, FILE* file);
void qbe_block_write(const QBEBlock* block, FILE* file);
void qbe_statement_write(const QBEStatement* statement, FILE* file);
void qbe_instruction_write(const QBEInstruction* instruction, FILE* file);

#endif

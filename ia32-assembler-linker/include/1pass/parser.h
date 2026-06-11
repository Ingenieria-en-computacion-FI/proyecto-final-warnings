#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"


typedef enum {
    OP_NONE,
    OP_REGISTER,
    OP_IMMEDIATE,
    OP_DIRECT_MEMORY,
    OP_INDIRECT_MEMORY 
} OperandType;


typedef struct {
    OperandType type;
    char base_reg[16];
    char index_reg[16];
    int scale;
    int displacement;
    int immediate_val;
    char label_ref[64];
} Operand;


typedef struct {
    char label[64];       
    char mnemonic[16];    
    Operand op1;          
    Operand op2;          
    int line;
} Instruction;


void parser_init(Token* tokens_array, int count);
Instruction parse_next_instruction(void);
int parser_has_more_tokens(void);
void parser_reset(void);

#endif
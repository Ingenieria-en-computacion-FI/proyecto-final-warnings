#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../include/1pass/parser.h"

static Token* token_stream;
static int total_tokens;
static int current_index;
static Token current_token;

static void advance(void) {
    if (current_index < total_tokens) {
        current_token = token_stream[current_index];
        current_index++;
    } else {
        current_token.type = TOKEN_EOF;
    }
}

void parser_init(Token* tokens_array, int count) {
    token_stream = tokens_array;
    total_tokens = count;
    current_index = 0;
    advance();
}

int parser_has_more_tokens(void) {
    return current_token.type != TOKEN_EOF;
}

static Operand init_operand(void) {
    Operand op;
    op.type = OP_NONE;
    op.base_reg[0] = '\0';
    op.index_reg[0] = '\0';
    op.scale = 1;
    op.displacement = 0;
    op.immediate_val = 0;
    op.label_ref[0] = '\0';
    return op;
}

static Operand parse_operand(void) {
    Operand op = init_operand();

    if (current_token.type == TOKEN_REGISTER) {
        op.type = OP_REGISTER;
        strcpy(op.base_reg, current_token.lexeme);
        advance();
        return op;
    }

    if (current_token.type == TOKEN_NUMBER) {
        op.type = OP_IMMEDIATE;
        op.immediate_val = (int)strtol(current_token.lexeme, NULL, 0);
        advance();
        return op;
    }

    if (current_token.type == TOKEN_IDENTIFIER) {
        op.type = OP_DIRECT_MEMORY;
        strcpy(op.label_ref, current_token.lexeme);
        advance();
        return op;
    }

    if (current_token.type == TOKEN_LBRACKET) {
        advance(); 
        
        if (current_token.type == TOKEN_REGISTER) {
            strcpy(op.base_reg, current_token.lexeme);
            advance();
        }

        while (current_token.type != TOKEN_RBRACKET && current_token.type != TOKEN_EOF) {
            if (current_token.type == TOKEN_PLUS) {
                advance();
                if (current_token.type == TOKEN_REGISTER) {
                    strcpy(op.index_reg, current_token.lexeme);
                    advance();
                } else if (current_token.type == TOKEN_NUMBER) {
                    op.displacement = (int)strtol(current_token.lexeme, NULL, 0);
                    advance();
                }
            } else if (current_token.type == TOKEN_STAR) {
                advance();
                if (current_token.type == TOKEN_NUMBER) {
                    op.scale = (int)strtol(current_token.lexeme, NULL, 0);
                    advance();
                }
            } else {
                advance();
            }
        }
        
        if (current_token.type == TOKEN_RBRACKET) {
            advance();
        }
        
        op.type = OP_INDIRECT_MEMORY;
        return op;
    }

    advance();
    return op;
}

Instruction parse_next_instruction(void) {
    Instruction instr;
    instr.label[0] = '\0';
    instr.mnemonic[0] = '\0';
    instr.op1 = init_operand();
    instr.op2 = init_operand();
    
    int sync_line = current_token.line;
    instr.line = sync_line;

    if (current_token.type == TOKEN_IDENTIFIER || current_token.type == TOKEN_DIRECTIVE) {
        char temp[64];
        strcpy(temp, current_token.lexeme);
        
        if (current_index < total_tokens && token_stream[current_index].type == TOKEN_COLON) {
            strcpy(instr.label, temp);
            advance(); 
            advance(); 
            
            sync_line = current_token.line; 
            instr.line = sync_line; 
        }
    }

    if (current_token.type == TOKEN_INSTRUCTION || current_token.type == TOKEN_DIRECTIVE) {
        strcpy(instr.mnemonic, current_token.lexeme);
        sync_line = current_token.line; 
        instr.line = sync_line;
        advance();

        if (current_token.type != TOKEN_EOF && current_token.line == sync_line) {
            instr.op1 = parse_operand();
            
            if (current_token.type == TOKEN_COMMA) {
                advance();
                instr.op2 = parse_operand();
            }
        }
    }

    while (current_token.type != TOKEN_EOF && current_token.line == sync_line) {
        advance();
    }

    return instr;
}

void parser_reset(void) {
    current_index = 0;
    advance();
}

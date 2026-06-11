#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../include/1pass/encoder.h"
#include "../../include/1pass/parser.h"
#include "../../include/1pass/symtable.h"
#include "../../include/1pass/objfile.h"

unsigned char machine_code_buffer[8192];
int machine_code_size = 0;

void reset_encoder_buffer(void) {
    machine_code_size = 0;
}

void emit_byte(unsigned char b) {
    if (machine_code_size < 8192) {
        machine_code_buffer[machine_code_size++] = b;
    }
}

void emit_32bit(int val) {
    emit_byte(val & 0xFF); 
    emit_byte((val >> 8) & 0xFF);
    emit_byte((val >> 16) & 0xFF); 
    emit_byte((val >> 24) & 0xFF);
}

unsigned char build_modrm(unsigned char mod, unsigned char reg, unsigned char rm) {
    return (mod << 6) | (reg << 3) | rm;
}

unsigned char build_sib(unsigned char scale, unsigned char index, unsigned char base) {
    return (scale << 6) | (index << 3) | base;
}

int get_register_code(const char* reg_name) {
    if (strcmp(reg_name, "EAX") == 0) return 0;
    if (strcmp(reg_name, "ECX") == 0) return 1;
    if (strcmp(reg_name, "EDX") == 0) return 2;
    if (strcmp(reg_name, "EBX") == 0) return 3;
    if (strcmp(reg_name, "ESP") == 0) return 4;
    if (strcmp(reg_name, "EBP") == 0) return 5;
    if (strcmp(reg_name, "ESI") == 0) return 6;
    if (strcmp(reg_name, "EDI") == 0) return 7;
    return -1;
}

void encode_instruction(Instruction* inst, int current_address) {
    if (inst == NULL || inst->mnemonic[0] == '\0') {
        return;
    }

    if (strcmp(inst->mnemonic, "MOV") == 0 && inst->op1.type == OP_REGISTER && inst->op2.type == OP_REGISTER) {
        unsigned char opcode = 0x89; 
        int dest_reg = get_register_code(inst->op1.base_reg);
        int src_reg = get_register_code(inst->op2.base_reg);
        emit_byte(opcode);
        emit_byte(build_modrm(3, src_reg, dest_reg)); 
        return;
    }

    if (strcmp(inst->mnemonic, "MOV") == 0 && inst->op1.type == OP_REGISTER && inst->op2.type == OP_IMMEDIATE) {
        unsigned char opcode = 0xB8 + get_register_code(inst->op1.base_reg);
        emit_byte(opcode);
        emit_32bit(inst->op2.immediate_val);
        return;
    }

    if ((strcmp(inst->mnemonic, "ADD") == 0 || strcmp(inst->mnemonic, "CMP") == 0) && 
        inst->op1.type == OP_REGISTER && inst->op2.type == OP_REGISTER) {
        unsigned char opcode = (strcmp(inst->mnemonic, "ADD") == 0) ? 0x01 : 0x39;
        int dest_reg = get_register_code(inst->op1.base_reg);
        int src_reg = get_register_code(inst->op2.base_reg);
        emit_byte(opcode);
        emit_byte(build_modrm(3, src_reg, dest_reg));
        return;
    }

    if ((strcmp(inst->mnemonic, "ADD") == 0 || strcmp(inst->mnemonic, "CMP") == 0) && 
        inst->op1.type == OP_REGISTER && inst->op2.type == OP_IMMEDIATE) {
        unsigned char opcode = 0x81;
        int dest_reg = get_register_code(inst->op1.base_reg);
        int reg_field = (strcmp(inst->mnemonic, "ADD") == 0) ? 0 : 7; 
        emit_byte(opcode);
        emit_byte(build_modrm(3, reg_field, dest_reg));
        emit_32bit(inst->op2.immediate_val);
        return;
    }

    if (strcmp(inst->mnemonic, "INC") == 0 && inst->op1.type == OP_REGISTER) {
        unsigned char opcode = 0x40 + get_register_code(inst->op1.base_reg);
        emit_byte(opcode);
        return;
    }

    if ((strcmp(inst->mnemonic, "JMP") == 0 || strcmp(inst->mnemonic, "CALL") == 0) && inst->op1.type == OP_DIRECT_MEMORY) {
        Symbol* sym = symtable_find(inst->op1.label_ref);
        if (sym != NULL) {
            int relative_offset = sym->address - (current_address + 5);
            emit_byte((strcmp(inst->mnemonic, "JMP") == 0) ? 0xE9 : 0xE8);
            emit_32bit(relative_offset);
            return;
        }
    }

    if (strcmp(inst->mnemonic, "JE") == 0 && inst->op1.type == OP_DIRECT_MEMORY) {
        Symbol* sym = symtable_find(inst->op1.label_ref);
        if (sym != NULL) {
            int relative_offset = sym->address - (current_address + 6);
            emit_byte(0x0F);
            emit_byte(0x84);
            emit_32bit(relative_offset);
            return;
        }
    }

    if (strcmp(inst->mnemonic, "RET") == 0) {
        emit_byte(0xC3);
        return;
    }

    if (strcmp(inst->mnemonic, "PUSH") == 0 && inst->op1.type == OP_REGISTER) {
        unsigned char opcode = 0x50 + get_register_code(inst->op1.base_reg);
        emit_byte(opcode);
        return;
    }

    if (strcmp(inst->mnemonic, "POP") == 0 && inst->op1.type == OP_REGISTER) {
        unsigned char opcode = 0x58 + get_register_code(inst->op1.base_reg);
        emit_byte(opcode);
        return;
    }

    if (strcmp(inst->mnemonic, "NOP") == 0) {
        emit_byte(0x90);
        return;
    }
}

int get_instruction_size(Instruction* inst) {
    if (inst == NULL || inst->mnemonic[0] == '\0') {
        return 0;
    }

    if (strcmp(inst->mnemonic, "SECTION") == 0 || 
        strcmp(inst->mnemonic, "GLOBAL")  == 0 ||
        strcmp(inst->mnemonic, "EXTERN")  == 0 ||
        strcmp(inst->mnemonic, "ORG")     == 0 ||
        strcmp(inst->mnemonic, "EQU")     == 0) {
        return 0;
    }

    if (strcmp(inst->mnemonic, "RESB") == 0) return inst->op1.immediate_val;
    if (strcmp(inst->mnemonic, "RESW") == 0) return inst->op1.immediate_val * 2;
    if (strcmp(inst->mnemonic, "RESD") == 0) return inst->op1.immediate_val * 4;

    if (strcmp(inst->mnemonic, "DB") == 0) {
        if (inst->op1.type == OP_IMMEDIATE) return 1;
    }
    if (strcmp(inst->mnemonic, "DW") == 0) return 2;
    if (strcmp(inst->mnemonic, "DD") == 0) return 4;

    if (strcmp(inst->mnemonic, "MOV") == 0) {
        if (inst->op1.type == OP_REGISTER && inst->op2.type == OP_REGISTER) return 2;
        if (inst->op1.type == OP_REGISTER && inst->op2.type == OP_IMMEDIATE) return 5;
        
        if (inst->op1.type == OP_REGISTER && inst->op2.type == OP_INDIRECT_MEMORY) {
            int disp = inst->op2.displacement;
            int mod = (disp == 0) ? 0 : (disp >= -128 && disp <= 127) ? 1 : 2;
            int size = 3;
            if (mod == 1) size += 1;
            else if (mod == 2) size += 4;
            return size;
        }
        
        if (inst->op1.type == OP_REGISTER && inst->op2.type == OP_DIRECT_MEMORY) {
            if (inst->op2.label_ref[0] == '\0') {
                int reg = get_register_code(inst->op1.base_reg);
                return (reg == 0) ? 5 : 6;
            }
        }
    }

    if (strcmp(inst->mnemonic, "LEA") == 0 && inst->op1.type == OP_REGISTER && inst->op2.type == OP_INDIRECT_MEMORY) {
        int disp = inst->op2.displacement;
        int mod = (disp == 0) ? 0 : (disp >= -128 && disp <= 127) ? 1 : 2;
        int size = 3;
        if (mod == 1) size += 1;
        else if (mod == 2) size += 4;
        return size;
    }

    if (strcmp(inst->mnemonic, "PUSH") == 0) {
        if (inst->op1.type == OP_REGISTER) return 1;
        if (inst->op1.type == OP_IMMEDIATE) return 5;
    }
    if (strcmp(inst->mnemonic, "POP") == 0 && inst->op1.type == OP_REGISTER) {
        return 1;
    }

    if (strcmp(inst->mnemonic, "ADD") == 0 || strcmp(inst->mnemonic, "OR")  == 0 || 
        strcmp(inst->mnemonic, "SUB") == 0 || strcmp(inst->mnemonic, "AND") == 0 || 
        strcmp(inst->mnemonic, "XOR") == 0 || strcmp(inst->mnemonic, "CMP") == 0) {
        
        if (inst->op1.type == OP_REGISTER) {
            if (inst->op2.type == OP_REGISTER) return 2;
            if (inst->op2.type == OP_IMMEDIATE) return 3;
        }
    }

    if (inst->op1.type == OP_REGISTER) {
        if (strcmp(inst->mnemonic, "INC") == 0 || strcmp(inst->mnemonic, "DEC") == 0) {
            return 1;
        }
        if (strcmp(inst->mnemonic, "NOT") == 0 || strcmp(inst->mnemonic, "NEG") == 0 || 
            strcmp(inst->mnemonic, "MUL") == 0 || strcmp(inst->mnemonic, "DIV") == 0) {
            return 2;
        }
    }

    if (strcmp(inst->mnemonic, "RET") == 0 || strcmp(inst->mnemonic, "NOP") == 0) {
        return 1;
    }

    if (strcmp(inst->mnemonic, "JMP") == 0 || strcmp(inst->mnemonic, "CALL") == 0) {
        return 5;
    }

    if (strcmp(inst->mnemonic, "JE")  == 0 || strcmp(inst->mnemonic, "JNE") == 0 ||
        strcmp(inst->mnemonic, "JG")  == 0 || strcmp(inst->mnemonic, "JL")  == 0 ||
        strcmp(inst->mnemonic, "JGE") == 0 || strcmp(inst->mnemonic, "JLE") == 0) {
        return 6;
    }

    if (strcmp(inst->mnemonic, "INT") == 0 && inst->op1.type == OP_IMMEDIATE) {
        return 2;
    }

    return 0;
}
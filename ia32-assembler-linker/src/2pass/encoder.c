#include <stdio.h>
#include <string.h>
#include "../../include/2pass/encoder.h"
#include "../../include/2pass/symtable.h"

extern int current_section;


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


void emit_byte_to_section(unsigned char byte, int section, unsigned char* t_buf, int* t_size, unsigned char* d_buf, int* d_size) {
    if (section == 1) { 
        t_buf[*t_size] = byte;
        (*t_size)++;
    } else if (section == 2) { 
        d_buf[*d_size] = byte;
        (*d_size)++;
    }
    printf("%02X ", byte);
}

void emit_32bit_to_section(int val, int section, unsigned char* t_buf, int* t_size, unsigned char* d_buf, int* d_size) {
    emit_byte_to_section(val & 0xFF, section, t_buf, t_size, d_buf, d_size);
    emit_byte_to_section((val >> 8) & 0xFF, section, t_buf, t_size, d_buf, d_size);
    emit_byte_to_section((val >> 16) & 0xFF, section, t_buf, t_size, d_buf, d_size);
    emit_byte_to_section((val >> 24) & 0xFF, section, t_buf, t_size, d_buf, d_size);
}

void encode_instruction(Instruction* inst, int current_address, unsigned char* text_buffer, int* text_size, unsigned char* data_buffer, int* data_size) {
    if (inst->op1.type == OP_DIRECT_MEMORY) {
        Symbol* sym = symtable_find(inst->op1.label_ref);
        if (sym != NULL && sym->defined == 1 && sym->section_id == 0) {
            inst->op1.type = OP_IMMEDIATE;
            inst->op1.immediate_val = sym->address;
        }
    }
    if (inst->op2.type == OP_DIRECT_MEMORY) {
        Symbol* sym = symtable_find(inst->op2.label_ref);
        if (sym != NULL && sym->defined == 1 && sym->section_id == 0) {
            inst->op2.type = OP_IMMEDIATE;
            inst->op2.immediate_val = sym->address;
        }
    }
    int disp = 0;
    int mod = 0;
    int scale_bits = 0;
    int reg_dst = 0;
    int op_ext = -1;
    int reg_code = 0;
    int target_dest = 0;
    int relative_offset = 0;
    unsigned char opcode = 0;
    unsigned char modrm = 0;
    unsigned char sib = 0;
    unsigned char base_op = 0x01;
    unsigned char modrm_ext = 0xFF;
    unsigned char jcc_op = 0x00;

    if (strcmp(inst->mnemonic, "SECTION") == 0 || 
        strcmp(inst->mnemonic, "GLOBAL")  == 0 ||
        strcmp(inst->mnemonic, "EXTERN")  == 0 ||
        strcmp(inst->mnemonic, "RESB")    == 0 ||
        strcmp(inst->mnemonic, "RESW")    == 0 ||
        strcmp(inst->mnemonic, "RESD")    == 0 ||
        strcmp(inst->mnemonic, "ORG")     == 0 ||
        strcmp(inst->mnemonic, "EQU")     == 0) {
        return;
    }

    if (strcmp(inst->mnemonic, "MOV") == 0) {
        if (inst->op1.type == OP_REGISTER && inst->op2.type == OP_REGISTER) {
            opcode = 0x89; 
            modrm = build_modrm(3, get_register_code(inst->op2.base_reg), get_register_code(inst->op1.base_reg));
            printf("  -> [Encoder] Codigo Maquina: ");
            emit_byte_to_section(opcode, current_section, text_buffer, text_size, data_buffer, data_size);
            emit_byte_to_section(modrm, current_section, text_buffer, text_size, data_buffer, data_size);
            printf("\n");
            return;
        }
        if (inst->op1.type == OP_REGISTER && inst->op2.type == OP_IMMEDIATE) {
            opcode = 0xB8 + get_register_code(inst->op1.base_reg); 
            printf("  -> [Encoder] Codigo Maquina: ");
            emit_byte_to_section(opcode, current_section, text_buffer, text_size, data_buffer, data_size);
            emit_32bit_to_section(inst->op2.immediate_val, current_section, text_buffer, text_size, data_buffer, data_size);
            printf("\n");
            return;
        }
        if (inst->op1.type == OP_REGISTER && inst->op2.type == OP_INDIRECT_MEMORY) {
            opcode = 0x8B;
            disp = inst->op2.displacement;
            mod = (disp == 0) ? 0 : (disp >= -128 && disp <= 127) ? 1 : 2;
            
            modrm = build_modrm(mod, get_register_code(inst->op1.base_reg), 4); // rm = 4 exige SIB
            
            scale_bits = (inst->op2.scale == 2) ? 1 : (inst->op2.scale == 4) ? 2 : (inst->op2.scale == 8) ? 3 : 0;
            sib = build_sib(scale_bits, get_register_code(inst->op2.index_reg), get_register_code(inst->op2.base_reg));
            
            printf("  -> [Encoder] Codigo Maquina: ");
            emit_byte_to_section(opcode, current_section, text_buffer, text_size, data_buffer, data_size);
            emit_byte_to_section(modrm, current_section, text_buffer, text_size, data_buffer, data_size);
            emit_byte_to_section(sib, current_section, text_buffer, text_size, data_buffer, data_size);
            
            if (mod == 1) emit_byte_to_section((unsigned char)(disp & 0xFF), current_section, text_buffer, text_size, data_buffer, data_size);
            else if (mod == 2) emit_32bit_to_section(disp, current_section, text_buffer, text_size, data_buffer, data_size);
            
            printf("\n");
            return;
        }
        if (inst->op1.type == OP_REGISTER && inst->op2.type == OP_DIRECT_MEMORY) {
            if (inst->op2.label_ref[0] == '\0') {
                printf("  -> [Encoder] Codigo Maquina: ");
                int reg = get_register_code(inst->op1.base_reg);
                if (reg == 0) { 
                    emit_byte_to_section(0xA1, current_section, text_buffer, text_size, data_buffer, data_size);
                } else {
                    emit_byte_to_section(0x8B, current_section, text_buffer, text_size, data_buffer, data_size);
                    emit_byte_to_section(build_modrm(0, reg, 5), current_section, text_buffer, text_size, data_buffer, data_size);
                }
                emit_32bit_to_section(inst->op2.displacement, current_section, text_buffer, text_size, data_buffer, data_size);
                printf("\n");
                return;
            }
        }
    }

    if (strcmp(inst->mnemonic, "PUSH") == 0) {
        if (inst->op1.type == OP_REGISTER) {
            opcode = 0x50 + get_register_code(inst->op1.base_reg);
            printf("  -> [Encoder] Codigo Maquina: ");
            emit_byte_to_section(opcode, current_section, text_buffer, text_size, data_buffer, data_size);
            printf("\n");
            return;
        }
        if (inst->op1.type == OP_IMMEDIATE) {
            printf("  -> [Encoder] Codigo Maquina: ");
            emit_byte_to_section(0x68, current_section, text_buffer, text_size, data_buffer, data_size);
            emit_32bit_to_section(inst->op1.immediate_val, current_section, text_buffer, text_size, data_buffer, data_size);
            printf("\n");
            return;
        }
    }

    if (strcmp(inst->mnemonic, "POP") == 0 && inst->op1.type == OP_REGISTER) {
        opcode = 0x58 + get_register_code(inst->op1.base_reg);
        printf("  -> [Encoder] Codigo Maquina: ");
        emit_byte_to_section(opcode, current_section, text_buffer, text_size, data_buffer, data_size);
        printf("\n");
        return;
    }

    if (strcmp(inst->mnemonic, "LEA") == 0 && inst->op1.type == OP_REGISTER && inst->op2.type == OP_INDIRECT_MEMORY) {
        opcode = 0x8D;
        disp = inst->op2.displacement;
        mod = (disp == 0) ? 0 : (disp >= -128 && disp <= 127) ? 1 : 2;
        modrm = build_modrm(mod, get_register_code(inst->op1.base_reg), 4);
        scale_bits = (inst->op2.scale == 2) ? 1 : (inst->op2.scale == 4) ? 2 : (inst->op2.scale == 8) ? 3 : 0;
        sib = build_sib(scale_bits, get_register_code(inst->op2.index_reg), get_register_code(inst->op2.base_reg));
        
        printf("  -> [Encoder] Codigo Maquina: ");
        emit_byte_to_section(opcode, current_section, text_buffer, text_size, data_buffer, data_size);
        emit_byte_to_section(modrm, current_section, text_buffer, text_size, data_buffer, data_size);
        emit_byte_to_section(sib, current_section, text_buffer, text_size, data_buffer, data_size);
        
        if (mod == 1) emit_byte_to_section((unsigned char)(disp & 0xFF), current_section, text_buffer, text_size, data_buffer, data_size);
        else if (mod == 2) emit_32bit_to_section(disp, current_section, text_buffer, text_size, data_buffer, data_size);
        
        printf("\n");
        return;
    }

  
    if (inst->op1.type == OP_REGISTER) {
        reg_dst = get_register_code(inst->op1.base_reg);
        op_ext = -1;
        base_op = 0x01;

        if (strcmp(inst->mnemonic, "ADD") == 0) { op_ext = 0; base_op = 0x01; }
        else if (strcmp(inst->mnemonic, "OR") == 0)  { op_ext = 1; base_op = 0x09; }
        else if (strcmp(inst->mnemonic, "SUB") == 0) { op_ext = 5; base_op = 0x29; }
        else if (strcmp(inst->mnemonic, "AND") == 0) { op_ext = 4; base_op = 0x21; }
        else if (strcmp(inst->mnemonic, "XOR") == 0) { op_ext = 6; base_op = 0x31; }
        else if (strcmp(inst->mnemonic, "CMP") == 0) { op_ext = 7; base_op = 0x39; }

        if (op_ext != -1) {
            if (inst->op2.type == OP_REGISTER) {
                modrm = build_modrm(3, get_register_code(inst->op2.base_reg), reg_dst);
                printf("  -> [Encoder] Codigo Maquina: ");
                emit_byte_to_section(base_op, current_section, text_buffer, text_size, data_buffer, data_size);
                emit_byte_to_section(modrm, current_section, text_buffer, text_size, data_buffer, data_size);
                printf("\n");
                return;
            }
            if (inst->op2.type == OP_IMMEDIATE) {
                modrm = build_modrm(3, op_ext, reg_dst);
                printf("  -> [Encoder] Codigo Maquina: ");
                emit_byte_to_section(0x83, current_section, text_buffer, text_size, data_buffer, data_size);
                emit_byte_to_section(modrm, current_section, text_buffer, text_size, data_buffer, data_size);
                emit_byte_to_section((unsigned char)(inst->op2.immediate_val & 0xFF), current_section, text_buffer, text_size, data_buffer, data_size);
                printf("\n");
                return;
            }
        }
    }

    if (inst->op1.type == OP_REGISTER) {
        reg_code = get_register_code(inst->op1.base_reg);
        
        if (strcmp(inst->mnemonic, "INC") == 0) {
            printf("  -> [Encoder] Codigo Maquina: ");
            emit_byte_to_section(0x40 + reg_code, current_section, text_buffer, text_size, data_buffer, data_size);
            printf("\n");
            return;
        }
        if (strcmp(inst->mnemonic, "DEC") == 0) {
            printf("  -> [Encoder] Codigo Maquina: ");
            emit_byte_to_section(0x48 + reg_code, current_section, text_buffer, text_size, data_buffer, data_size);
            printf("\n");
            return;
        }
        
        modrm_ext = 0xFF;
        if (strcmp(inst->mnemonic, "NOT") == 0) modrm_ext = 2;
        else if (strcmp(inst->mnemonic, "NEG") == 0) modrm_ext = 3;
        else if (strcmp(inst->mnemonic, "MUL") == 0) modrm_ext = 4;
        else if (strcmp(inst->mnemonic, "DIV") == 0) modrm_ext = 6;

        if (modrm_ext != 0xFF) {
            modrm = build_modrm(3, modrm_ext, reg_code);
            printf("  -> [Encoder] Codigo Maquina: ");
            emit_byte_to_section(0xF7, current_section, text_buffer, text_size, data_buffer, data_size);
            emit_byte_to_section(modrm, current_section, text_buffer, text_size, data_buffer, data_size);
            printf("\n");
            return;
        }
    }


    if (strcmp(inst->mnemonic, "RET") == 0) {
        printf("  -> [Encoder] Codigo Maquina: ");
        emit_byte_to_section(0xC3, current_section, text_buffer, text_size, data_buffer, data_size);
        printf("\n");
        return;
    }
    if (strcmp(inst->mnemonic, "NOP") == 0) {
        printf("  -> [Encoder] Codigo Maquina: ");  
        emit_byte_to_section(0x90, current_section, text_buffer, text_size, data_buffer, data_size);
        printf("\n");
        return;
    }

    if (strcmp(inst->mnemonic, "JMP") == 0 || strcmp(inst->mnemonic, "CALL") == 0 || 
        strcmp(inst->mnemonic, "JE")  == 0 || strcmp(inst->mnemonic, "JNE")  == 0 ||
        strcmp(inst->mnemonic, "JG")  == 0 || strcmp(inst->mnemonic, "JL")   == 0 ||
        strcmp(inst->mnemonic, "JGE") == 0 || strcmp(inst->mnemonic, "JLE")  == 0) {
        
        if (inst->op1.type == OP_DIRECT_MEMORY) { 
            Symbol* sym = symtable_find(inst->op1.label_ref);
            
            if (sym != NULL && sym->defined == 1) {
                target_dest = sym->address;
            } else {
                if (strcmp(inst->mnemonic, "CALL") == 0) {
                    printf("  -> [Encoder] Codigo Maquina: ");
                    emit_byte_to_section(0xE8, current_section, text_buffer, text_size, data_buffer, data_size);
                    reltable_add(*text_size, inst->op1.label_ref);
                    emit_32bit_to_section(0, current_section, text_buffer, text_size, data_buffer, data_size);
                    printf(" (Relocacion pendiente para %s)\n", inst->op1.label_ref);
                    return;
                }
                printf("  -> [Encoder Error] Etiqueta '%s' no definida en este archivo.\n", inst->op1.label_ref);
                return;
            }

            if (strcmp(inst->mnemonic, "JMP") == 0) {
                relative_offset = target_dest - (current_address + 5);
                printf("  -> [Encoder] Codigo Maquina: ");
                emit_byte_to_section(0xE9, current_section, text_buffer, text_size, data_buffer, data_size);
                emit_32bit_to_section(relative_offset, current_section, text_buffer, text_size, data_buffer, data_size);
                printf("\n");
                return;
            }
            if (strcmp(inst->mnemonic, "CALL") == 0) {
                relative_offset = target_dest - (current_address + 5);
                printf("  -> [Encoder] Codigo Maquina: ");
                emit_byte_to_section(0xE8, current_section, text_buffer, text_size, data_buffer, data_size);
                emit_32bit_to_section(relative_offset, current_section, text_buffer, text_size, data_buffer, data_size);
                printf("\n");
                return;
            }

            jcc_op = 0x00;
            if (strcmp(inst->mnemonic, "JE") == 0) jcc_op = 0x84;
            else if (strcmp(inst->mnemonic, "JNE") == 0) jcc_op = 0x85;
            else if (strcmp(inst->mnemonic, "JL")  == 0) jcc_op = 0x8C;
            else if (strcmp(inst->mnemonic, "JGE") == 0) jcc_op = 0x8D;
            else if (strcmp(inst->mnemonic, "JG")  == 0) jcc_op = 0x8F;
            else if (strcmp(inst->mnemonic, "JLE") == 0) jcc_op = 0x8E;

            if (jcc_op != 0x00) {
                relative_offset = target_dest - (current_address + 6);
                printf("  -> [Encoder] Codigo Maquina: ");
                emit_byte_to_section(0x0F, current_section, text_buffer, text_size, data_buffer, data_size);
                emit_byte_to_section(jcc_op, current_section, text_buffer, text_size, data_buffer, data_size);
                emit_32bit_to_section(relative_offset, current_section, text_buffer, text_size, data_buffer, data_size);
                printf("\n");
                return;
            }
        }
    }

   
    if (strcmp(inst->mnemonic, "INT") == 0 && inst->op1.type == OP_IMMEDIATE) {
        printf("  -> [Encoder] Codigo Maquina: ");
        emit_byte_to_section(0xCD, current_section, text_buffer, text_size, data_buffer, data_size);
        emit_byte_to_section((unsigned char)(inst->op1.immediate_val & 0xFF), current_section, text_buffer, text_size, data_buffer, data_size);
        printf("\n");
        return;
    }

    if (strcmp(inst->mnemonic, "DB") == 0) {
        printf("  -> [Encoder] Datos .data: ");
        if (inst->op1.type == OP_IMMEDIATE) {
            emit_byte_to_section((unsigned char)(inst->op1.immediate_val & 0xFF),
                                current_section, text_buffer, text_size, data_buffer, data_size);
        } else if (inst->op1.type == OP_STRING) {
            for (int i = 0; inst->op1.str_val[i] != '\0'; i++) {
                emit_byte_to_section((unsigned char)inst->op1.str_val[i],
                                    current_section, text_buffer, text_size, data_buffer, data_size);
            }
            emit_byte_to_section(0x00, current_section, text_buffer, text_size, data_buffer, data_size);
        }
        printf("\n");
        return;
    }

    if (strcmp(inst->mnemonic, "DW") == 0) {
        printf("  -> [Encoder] Datos .data: ");
        if (inst->op1.type == OP_IMMEDIATE) {
            emit_byte_to_section((unsigned char)(inst->op1.immediate_val & 0xFF),
                                current_section, text_buffer, text_size, data_buffer, data_size);
            emit_byte_to_section((unsigned char)((inst->op1.immediate_val >> 8) & 0xFF),
                                current_section, text_buffer, text_size, data_buffer, data_size);
        }
        printf("\n");
        return;
    }

    if (strcmp(inst->mnemonic, "DD") == 0) {
        printf("  -> [Encoder] Datos .data: ");
        if (inst->op1.type == OP_IMMEDIATE) {
            emit_32bit_to_section(inst->op1.immediate_val,
                                current_section, text_buffer, text_size, data_buffer, data_size);
        }
        printf("\n");
        return;
    }

    printf("  -> [Encoder Error] Instruccion o combinación no soportada.\n");
}
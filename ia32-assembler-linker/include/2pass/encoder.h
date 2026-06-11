#ifndef ENCODER_H
#define ENCODER_H

#include "parser.h" 

typedef enum {
    REG_EAX = 0, 
    REG_ECX = 1, 
    REG_EDX = 2, 
    REG_EBX = 3, 
    REG_ESP = 4, 
    REG_EBP = 5, 
    REG_ESI = 6, 
    REG_EDI = 7  
} IA32_Reg;

unsigned char build_modrm(unsigned char mod, unsigned char reg, unsigned char rm);
unsigned char build_sib(unsigned char scale, unsigned char index, unsigned char base);
int get_register_code(const char* reg_name);
void encode_instruction(Instruction* inst, int current_address, unsigned char* text_buffer, int* text_size, unsigned char* data_buffer, int* data_size);

#endif
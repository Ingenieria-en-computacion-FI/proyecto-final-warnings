#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/1pass/lexer.h"
#include "../../include/1pass/parser.h"
#include "../../include/1pass/symtable.h"
#include "../../include/1pass/encoder.h"
#include "../../include/1pass/fixup.h"
#include "../../include/1pass/objfile.h"

char* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: No se puede abrir el archivo '%s'\n", filename);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = (char*)malloc(length + 1);
    if (!buffer) {
        fprintf(stderr, "Error: Memoria insuficiente\n");
        fclose(file);
        return NULL;
    }
    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <archivo_ensamblador>\n", argv[0]);
        return 1;
    }

    char* source = read_file(argv[1]);
    if (!source) {
        return 1;
    }

    Lexer lexer;
    init_lexer(&lexer, source);
    
    int t_count;
    Token* tokens = tokenize(&lexer, &t_count);
    
    parser_init(tokens, t_count);
    symtable_init();
    reset_encoder_buffer();

    int loc_counter = 0x08048000;
    
    printf("=== Ensamblador de 1 Pasada ===\n");
    printf("Procesando archivo: %s\n\n", argv[1]);

    while (parser_has_more_tokens()) {
        Instruction inst = parse_next_instruction();
        
        if (inst.label[0] != '\0') {
            symtable_add(inst.label, loc_counter, 1);
            resolve_fixups(machine_code_buffer, inst.label, loc_counter);
            printf("[Etiqueta] %s -> 0x%08X\n", inst.label, loc_counter);
        }

        if (inst.mnemonic[0] != '\0') {
            printf("[Instrucción] %s", inst.mnemonic);
            
            if ((strcmp(inst.mnemonic, "JMP") == 0 || strcmp(inst.mnemonic, "CALL") == 0) && 
                inst.op1.type == OP_DIRECT_MEMORY) {
                
                Symbol* sym = symtable_find(inst.op1.label_ref);
                if (sym != NULL) {
                    printf(" %s", inst.op1.label_ref);
                    encode_instruction(&inst, loc_counter);
                } else {
                    printf(" %s (pendiente)", inst.op1.label_ref);
                    add_fixup(inst.op1.label_ref, machine_code_size, loc_counter);
                    if (strcmp(inst.mnemonic, "JMP") == 0) {
                        emit_byte(0xE9);
                    } else {
                        emit_byte(0xE8);
                    }
                    emit_32bit(0);  
                }
            } else {
                if (inst.op1.type != OP_NONE) {
                    printf(" %s", inst.op1.base_reg[0] != '\0' ? inst.op1.base_reg : 
                           inst.op1.label_ref[0] != '\0' ? inst.op1.label_ref : 
                           inst.op1.immediate_val ? "" : "");
                }
                if (inst.op2.type != OP_NONE) {
                    printf(", %s", inst.op2.base_reg[0] != '\0' ? inst.op2.base_reg : 
                           inst.op2.label_ref[0] != '\0' ? inst.op2.label_ref : "");
                }
                encode_instruction(&inst, loc_counter);
            }
            
            int size = get_instruction_size(&inst);
            printf(" (%d bytes @ 0x%08X)\n", size, loc_counter);
            loc_counter += size;
        }
    }

    printf("\n=== Compilación Completada ===\n");
    printf("Código máquina generado: %d bytes\n", machine_code_size);
    
    symtable_print();
    
    write_object_file("salida_1pasada.o");
    
    free(tokens);
    free(source);
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../../include/2pass/lexer.h"
#include "../../include/2pass/parser.h"
#include "../../include/2pass/symtable.h"
#include "../../include/2pass/encoder.h"
#include "../../include/2pass/objfile.h"
#include "../../include/2pass/objwriter.h"
#include "../../include/2pass/linker.h"
extern int current_section;

int obtener_tamano_instruccion(Instruction* inst) {
    if (strcmp(inst->mnemonic, "RET") == 0 || strcmp(inst->mnemonic, "NOP") == 0) return 1;
    if (strcmp(inst->mnemonic, "INC") == 0 || strcmp(inst->mnemonic, "DEC") == 0) return 1;
    
    if (strcmp(inst->mnemonic, "MOV") == 0) {
        if (inst->op1.type == OP_REGISTER && inst->op2.type == OP_REGISTER) return 2; 
        if (inst->op1.type == OP_REGISTER && inst->op2.type == OP_IMMEDIATE) return 5; 
        if (inst->op1.type == OP_REGISTER && inst->op2.type == OP_INDIRECT_MEMORY) {
            int disp = inst->op2.displacement;
            int mod = (disp == 0) ? 0 : (disp >= -128 && disp <= 127) ? 1 : 2;
            if (mod == 0) return 3; 
            if (mod == 1) return 4; 
            return 7;               
        }
    }
    
    if (strcmp(inst->mnemonic, "JMP") == 0 || strcmp(inst->mnemonic, "CALL") == 0) return 5; 
    if (inst->mnemonic[0] == 'J') return 6; 
    
    if (strcmp(inst->mnemonic, "ADD") == 0 || strcmp(inst->mnemonic, "CMP") == 0 || 
        strcmp(inst->mnemonic, "SUB") == 0 || strcmp(inst->mnemonic, "AND") == 0 || 
        strcmp(inst->mnemonic, "OR") == 0  || strcmp(inst->mnemonic, "XOR") == 0) {
        if (inst->op2.type == OP_REGISTER) return 2;  
        if (inst->op2.type == OP_IMMEDIATE) return 3; 
    }
    
    if (strcmp(inst->mnemonic, "INT") == 0) return 2;
    return 0;
}

char* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return NULL;
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = (char*)malloc(length + 1);
    if (buffer) {
        size_t read_bytes = fread(buffer, 1, length, file);
        buffer[read_bytes] = '\0';
    }
    fclose(file);
    return buffer;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <archivo.asm>\n", argv[0]);
        fprintf(stderr, "Uso Linker:      %s --link <archivo1.o> <archivo2.o> ... -o <ejecutable>\n", argv[0]);
        return 1;
    }


    if (strcmp(argv[1], "--link") == 0) {
        const char* input_files[10];
        int file_count = 0;
        const char* output_file = "a.out"; 

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-o") == 0 && (i + 1 < argc)) {
                output_file = argv[i + 1];
                i++; 
            } else {
                if (file_count < 10) {
                    input_files[file_count++] = argv[i];
                }
            }
        }

        if (file_count == 0) {
            fprintf(stderr, "[Error] No se especificaron archivos objeto para enlazar.\n");
            return 1;
        }

        run_linker(input_files, file_count, output_file);
        return 0;
    }

    char* source_code = NULL;
    Lexer lexer;
    int token_count = 0;
    Token* tokens = NULL;
    
    int location_counter = 0;
    int prev_buffer_size = 0;
    unsigned char text_section_buffer[4096];
    unsigned char data_section_buffer[4096];
    int current_text_size = 0;
    int current_data_size = 0;
    
    memset(text_section_buffer, 0, sizeof(text_section_buffer));
    memset(data_section_buffer, 0, sizeof(data_section_buffer));

    char output_filename[256];
    strncpy(output_filename, argv[1], sizeof(output_filename) - 3);
    char* dot = strrchr(output_filename, '.');
    if (dot != NULL) strcpy(dot, ".o");
    else strcat(output_filename, ".o");

    source_code = read_file(argv[1]);
    if (!source_code) {
        fprintf(stderr, "Error: No se pudo leer el archivo %s\n", argv[1]);
        return 1;
    }

    printf("Procesando archivo fuente: %s\n\n", argv[1]);
    printf("Archivo objeto de salida:  %s\n\n", output_filename);

    init_lexer(&lexer, source_code);
    tokens = tokenize(&lexer, &token_count);

    printf("--- PASS 1: MAPEO DE MEMORIA (CONSTRUCCIÓN DE TABLA) ---\n");
    parser_init(tokens, token_count);
    symtable_init();

    int lc_text = 0;
    int lc_data = 0;
    int lc_bss = 0;
    
    while (parser_has_more_tokens()) {
        Instruction inst = parse_next_instruction();
        
        if (inst.label[0] != '\0') {
            Symbol* existing_sym = symtable_find(inst.label);
            
            if (existing_sym != NULL) {
                if (existing_sym->defined == 1) {
                    fprintf(stderr, "Error fatal: La etiqueta '%s' ya ha sido definida previamente.\n", inst.label);
                    exit(1);
                } else {
                    if (current_section == 1) { 
                        existing_sym->address = lc_text; existing_sym->section_id = 1; 
                    } else if (current_section == 2) { 
                        existing_sym->address = lc_data; existing_sym->section_id = 2; 
                    } else if (current_section == 3) { 
                        existing_sym->address = lc_bss; existing_sym->section_id = 3; 
                    }
                    
                    if (strcmp(inst.mnemonic, "EQU") == 0 && inst.op1.type == OP_IMMEDIATE) {
                        existing_sym->address = inst.op1.immediate_val;
                    }
                    existing_sym->defined = 1;
                }
            } else {
                if (strcmp(inst.mnemonic, "EQU") == 0 && inst.op1.type == OP_IMMEDIATE) {
                    symtable_add(inst.label, inst.op1.immediate_val, 0, 0, 0, 1);
                } else if (current_section == 1) {
                    symtable_add(inst.label, lc_text, 1, 0, 0, 1);
                } else if (current_section == 2) {
                    symtable_add(inst.label, lc_data, 2, 0, 0, 1);
                } else if (current_section == 3) {
                    symtable_add(inst.label, lc_bss, 3, 0, 0, 1);
                }
            }
        }
        if (strcmp(inst.mnemonic, "EXTERN") == 0 && inst.op1.type == OP_DIRECT_MEMORY) {
            symtable_add(inst.op1.label_ref, 0, 0, 0, 1, 0);
        }

        if (strcmp(inst.mnemonic, "GLOBAL") == 0 && inst.op1.type == OP_DIRECT_MEMORY) {
            Symbol* sym = symtable_find(inst.op1.label_ref);
            if (sym != NULL) {
                sym->is_global = 1;
            }
            else {
                symtable_add(inst.op1.label_ref, 0, 1, 1, 0, 0);
            }
        }

        if (inst.mnemonic[0] != '\0') {
            if (strcmp(inst.mnemonic, "ORG") == 0 && inst.op1.type == OP_IMMEDIATE) {
                if (current_section == 1) lc_text = inst.op1.immediate_val;
                else if (current_section == 2) lc_data = inst.op1.immediate_val;
                else if (current_section == 3) lc_bss  = inst.op1.immediate_val;
            
            } else if (current_section == 1) {
                lc_text += obtener_tamano_instruccion(&inst);
            } else if (current_section == 2) {
                if (strcmp(inst.mnemonic, "DB") == 0) {
                    if (inst.op1.type == OP_STRING)
                        lc_data += strlen(inst.op1.str_val) + 1; // +1 por null terminator
                    else
                        lc_data += 1;
                } else if (strcmp(inst.mnemonic, "DW") == 0) {
                    lc_data += 2;
                } else if (strcmp(inst.mnemonic, "DD") == 0) {
                    lc_data += 4;
                }
            } else if (current_section == 3) {
                if (strcmp(inst.mnemonic, "RESB") == 0) {
                    lc_bss += inst.op1.immediate_val;
                } else if (strcmp(inst.mnemonic, "RESW") == 0) {
                    lc_bss += inst.op1.immediate_val * 2;
                } else if (strcmp(inst.mnemonic, "RESD") == 0) {
                    lc_bss += inst.op1.immediate_val * 4;
                }
            }
        }
    }

    extern int current_section; 
    
    symtable_print();


    printf("\n--- PASS 2: TRADUCCION A LENGUAJE MAQUINA ---\n");
    parser_reset(); 
    location_counter = 0; 
    current_text_size = 0;
    memset(text_section_buffer, 0, sizeof(text_section_buffer));

    while (parser_has_more_tokens()) {
        Instruction inst = parse_next_instruction();
        
        if (inst.mnemonic[0] != '\0' || inst.label[0] != '\0') {
            printf("0x%08X -> Linea %d: ", location_counter, inst.line);
            if (inst.label[0] != '\0') printf("Etiqueta[%s] ", inst.label);
            if (inst.mnemonic[0] != '\0') printf("Instruccion[%s] ", inst.mnemonic);
            printf("\n");

            if (inst.mnemonic[0] != '\0') {
                prev_buffer_size = current_text_size;
                
                encode_instruction(&inst, location_counter, text_section_buffer, &current_text_size, data_section_buffer, &current_data_size);
                
                location_counter += (current_text_size - prev_buffer_size); 
            }
        }
    }

    generate_obj_file(output_filename, text_section_buffer, current_text_size, data_section_buffer, current_data_size, lc_bss, reltable, rel_count);

    free(tokens);
    free(source_code); 
    return 0;
}
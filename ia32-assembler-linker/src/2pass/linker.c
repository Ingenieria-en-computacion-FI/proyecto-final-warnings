#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>  
#include "../../include/2pass/objfile.h"
#include "../../include/2pass/linker.h"

GlobalSymbol global_symtable[MAX_GLOBAL_SYMBOLS];
int global_symbol_count = 0;

unsigned char output_text_buffer[MAX_OUTPUT_SIZE];
uint32_t total_text_size = 0;

unsigned char output_data_buffer[MAX_OUTPUT_SIZE];
uint32_t total_data_size = 0;

int find_global_symbol(const char* name) {
    for (int i = 0; i < global_symbol_count; i++) {
        if (strcmp(global_symtable[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

void add_global_symbol(const char* name, uint32_t final_address, uint32_t section_id, uint8_t is_defined) {
    int idx = find_global_symbol(name);
    if (idx != -1) {
        if (!global_symtable[idx].is_defined && is_defined) {
            global_symtable[idx].final_address = final_address;
            global_symtable[idx].section_id = section_id;
            global_symtable[idx].is_defined = 1;
        }
    } else if (global_symbol_count < MAX_GLOBAL_SYMBOLS) {
        strncpy(global_symtable[global_symbol_count].name, name, 31);
        global_symtable[global_symbol_count].final_address = final_address;
        global_symtable[global_symbol_count].section_id = section_id;
        global_symtable[global_symbol_count].is_defined = is_defined;
        global_symbol_count++;
    }
}

int run_linker(const char* input_filenames[], int file_count, const char* output_filename) {
    int i = 0;
    uint32_t s = 0;
    uint32_t r = 0;
    int sym_idx = -1;
    
    uint32_t file_text_offsets[MAX_INPUT_FILES];
    uint32_t file_data_offsets[MAX_INPUT_FILES];
    
    uint32_t sym_table_offset = 0;
    uint32_t rel_table_offset = 0;
    uint32_t patch_address = 0;
    uint32_t target_absolute_address = 0;
    
    int64_t large_offset = 0; 
    int32_t relative_offset = 0;
    
    FILE* f = NULL;
    FILE* out = NULL;
    
    ObjHeader header;
    ObjSymbol sym;
    ObjRelocation rel;

    printf("=== INICIANDO MINI LINKER (UNAM - FORMATO OBJETO) ===\n\n");

    memset(output_text_buffer, 0, sizeof(output_text_buffer));
    memset(output_data_buffer, 0, sizeof(output_data_buffer));
    memset(file_text_offsets, 0, sizeof(file_text_offsets));
    memset(file_data_offsets, 0, sizeof(file_data_offsets));
    
    total_text_size = 0;
    total_data_size = 0;

    for (i = 0; i < file_count; i++) {
        f = fopen(input_filenames[i], "rb");
        if (!f) {
            fprintf(stderr, "[Linker Error] No se pudo abrir el archivo objeto: %s\n", input_filenames[i]);
            return 0;
        }

        fread(&header, sizeof(ObjHeader), 1, f);

        if (strncmp(header.magic, "UNAM", 4) != 0) {
            fprintf(stderr, "[Linker Error] Firma magica 'UNAM' no encontrada en %s\n", input_filenames[i]);
            fclose(f);
            return 0;
        }

        file_text_offsets[i] = total_text_size;
        file_data_offsets[i] = total_data_size;

        if (header.text_size > 0) {
            fread(output_text_buffer + total_text_size, 1, header.text_size, f);
            total_text_size += header.text_size;
        }

        if (header.data_size > 0) {
            fread(output_data_buffer + total_data_size, 1, header.data_size, f);
            total_data_size += header.data_size;
        }

        sym_table_offset = sizeof(ObjHeader) + header.text_size + header.data_size;
        fseek(f, sym_table_offset, SEEK_SET);

        for (s = 0; s < header.sym_count; s++) {
            fread(&sym, sizeof(ObjSymbol), 1, f);

            uint32_t absolute_addr = 0;
            uint8_t defined = 0;

            if (!sym.is_extern) {
                defined = 1;
                if (sym.section_id == 1) {
                    absolute_addr = sym.address + file_text_offsets[i];
                } else if (sym.section_id == 2) {
                    absolute_addr = sym.address + file_data_offsets[i];
                } else {
                    absolute_addr = sym.address;
                }
            }

            add_global_symbol(sym.name, absolute_addr, sym.section_id, defined);
        }

        fclose(f);
    }

    for (i = 0; i < file_count; i++) {
        f = fopen(input_filenames[i], "rb");
        if (!f) continue;

        fread(&header, sizeof(ObjHeader), 1, f);

        rel_table_offset = sizeof(ObjHeader) + header.text_size + header.data_size + (header.sym_count * sizeof(ObjSymbol));
        fseek(f, rel_table_offset, SEEK_SET);

        for (r = 0; r < header.rel_count; r++) {
            fread(&rel, sizeof(ObjRelocation), 1, f);

            sym_idx = find_global_symbol(rel.symbol_name);
            if (sym_idx == -1 || !global_symtable[sym_idx].is_defined) {
                fprintf(stderr, "[Linker Error] Simbolo externo no resuelto: '%s'\n", rel.symbol_name);
                fclose(f);
                return 0;
            }

            patch_address = rel.offset + file_text_offsets[i];
            target_absolute_address = global_symtable[sym_idx].final_address;

            large_offset = (int64_t)target_absolute_address - (patch_address + 4);

            if (large_offset < INT_MIN || large_offset > INT_MAX) {
                fprintf(stderr, "[Fatal Linker Error]Distancia de salto demasiado grande para '%s'.\n", rel.symbol_name);
                fprintf(stderr, "El offset (%ld) desborda la arquitectura de 32 bits.\n", large_offset);
                fclose(f);
                return 0;
            }

            relative_offset = (int32_t)large_offset;

            output_text_buffer[patch_address]     = (relative_offset & 0xFF);
            output_text_buffer[patch_address + 1] = ((relative_offset >> 8) & 0xFF);
            output_text_buffer[patch_address + 2] = ((relative_offset >> 16) & 0xFF);
            output_text_buffer[patch_address + 3] = ((relative_offset >> 24) & 0xFF);
        }

        fclose(f);
    }


    out = fopen(output_filename, "wb");
    if (!out) {
        fprintf(stderr, "[Linker Error] Error fatal de escritura al crear '%s'\n", output_filename);
        return 0;
    }

    fwrite(output_text_buffer, 1, total_text_size, out);
    if (total_data_size > 0) {
        fwrite(output_data_buffer, 1, total_data_size, out);
    }
    
    fclose(out);

    printf("[Linker] Proceso de enlazado completado de manera exitosa.\n");
    printf("         -> Ejecutable '%s' generado: %u bytes de codigo | %u bytes de datos.\n", 
        output_filename, total_text_size, total_data_size);
    return 1;
}
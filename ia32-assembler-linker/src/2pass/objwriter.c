#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/2pass/objfile.h"
#include "../../include/2pass/symtable.h"

int generate_obj_file(const char* filename, unsigned char* t_buf, int t_size, unsigned char* d_buf, int d_size, uint32_t bss, ObjRelocation* rels, int count) {
    FILE* out = fopen(filename, "wb");
    if (!out) {
        fprintf(stderr, "Error: No se pudo crear el archivo objeto %s\n", filename);
        return 0;
    }

    ObjHeader header;
    memset(&header, 0, sizeof(ObjHeader)); 
    memcpy(header.magic, "UNAM", 4);
    header.text_size = t_size;
    header.data_size = d_size;                 
    header.bss_size = bss;                   
    header.sym_count = symbol_count;      
    header.rel_count = rel_count;               

    fwrite(&header, sizeof(ObjHeader), 1, out);
    if (t_size > 0) {
        fwrite(t_buf, 1, t_size, out);
    }
    if (d_size > 0 && d_buf != NULL) {
        fwrite(d_buf, 1, d_size, out);
    }

    for (int i = 0; i < symbol_count; i++) {
        ObjSymbol export_sym;
        memset(&export_sym, 0, sizeof(ObjSymbol));
        
        strncpy(export_sym.name, symtable[i].name, 31);
        export_sym.address = symtable[i].address;
        export_sym.section_id = symtable[i].section_id;
        export_sym.is_global = symtable[i].is_global;
        export_sym.is_extern = symtable[i].is_extern;
        
        fwrite(&export_sym, sizeof(ObjSymbol), 1, out);
    }
    if (count > 0 && rels != NULL) {
        fwrite(rels, sizeof(ObjRelocation), count, out);
    }
    fclose(out);
    printf("\n[Linker] Archivo objeto '%s' generado exitosamente con %d bytes de codigo maquina.\n", filename, t_size);
    return 1;
}
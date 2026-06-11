#include <stdio.h>
#include <string.h>
#include "../../include/1pass/fixup.h"

Fixup fixup_table[256];
int fixup_count = 0;

void add_fixup(const char* label, int offset, int inst_addr) {
    if (fixup_count >= 256) {
        fprintf(stderr, "Error: Tabla de fixups llena\n");
        return;
    }
    strcpy(fixup_table[fixup_count].label_name, label);
    fixup_table[fixup_count].buffer_offset = offset;
    fixup_table[fixup_count].instruction_address = inst_addr;
    fixup_count++;
}

void resolve_fixups(unsigned char* buffer, const char* resolved_label, int target_address) {
    for (int i = 0; i < fixup_count; i++) {
        if (strcmp(fixup_table[i].label_name, resolved_label) == 0) {
            int relative_offset = target_address - (fixup_table[i].instruction_address + 5);
            int idx = fixup_table[i].buffer_offset;
            buffer[idx]     = (relative_offset & 0xFF);
            buffer[idx + 1] = ((relative_offset >> 8) & 0xFF);
            buffer[idx + 2] = ((relative_offset >> 16) & 0xFF);
            buffer[idx + 3] = ((relative_offset >> 24) & 0xFF);
            printf("  -> [Fixup] Etiqueta '%s' resuelta en offset %d\n", resolved_label, idx);
        }
    }
}

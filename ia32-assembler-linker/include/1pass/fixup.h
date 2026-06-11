#ifndef FIXUP_H
#define FIXUP_H

typedef struct {
    char label_name[64];
    int buffer_offset;      
    int instruction_address; 
} Fixup;

void add_fixup(const char* label, int offset, int inst_addr);
void resolve_fixups(unsigned char* buffer, const char* resolved_label, int target_address);

#endif
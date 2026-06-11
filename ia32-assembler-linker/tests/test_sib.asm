; test_sib.asm
; Cubre: Modos de direccionamiento avanzados, memoria directa y SIB

SECTION .text
GLOBAL _start

_start:
    MOV EBX, 0x1000
    MOV ECX, 2
    
    MOV EAX, [1000]

    MOV EDX, [EBP+4]

    MOV ESI, [EBX+ECX]

    MOV EAX, [EBX+ECX*4+8]
    
    INT 0x80
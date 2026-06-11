; test_saltos.asm
; Cubre: CMP, JMP, JE, JG y Referencias Adelantadas

SECTION .text
GLOBAL _start

_start:
    MOV EAX, 10
    CMP EAX, 10

    JE salto_adelante  
    
    INC EAX
    
salto_adelante:
    MOV EBX, 5
    CMP EAX, EBX
    JG fin_programa
    
    DEC EBX

    JNE no_igual
    JL menor
    JGE mayor_igual
    JLE menor_igual
    JMP fin_programa

no_igual:
    INC EAX
    JMP fin_programa
menor:
    DEC EAX
    JMP fin_programa
mayor_igual:
    MOV ECX, EAX
    JMP fin_programa
menor_igual:
    MOV EDX, EAX
    JMP fin_programa
    
fin_programa:
    INT 0x80
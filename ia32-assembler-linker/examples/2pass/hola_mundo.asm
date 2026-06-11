
SECTION .data
    cadena DB "Iniciando sistema..."  
    valor_a DW 15
    valor_b DW 25

SECTION .text
GLOBAL _start

_start:
    MOV EAX, 10
    ADD EAX, 5
    SUB EAX, 2
    
    MOV EBX, EAX
    INC EBX

    MOV EAX, 1         
    MOV EBX, 0          
    INT 0x80            
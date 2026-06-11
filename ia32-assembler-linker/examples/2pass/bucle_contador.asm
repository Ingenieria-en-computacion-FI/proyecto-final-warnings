
SECTION .text
GLOBAL _start

_start:
    MOV ECX, 5         
    MOV EAX, 0          
inicio_ciclo:
    ADD EAX, 10        
    DEC ECX             
    
    CMP ECX, 0         
    JG inicio_ciclo     

   
    MOV EAX, 1
    MOV EBX, 0
    INT 0x80
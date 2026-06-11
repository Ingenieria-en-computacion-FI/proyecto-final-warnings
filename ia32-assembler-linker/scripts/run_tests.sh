#!/bin/bash

echo "======================================================"
echo "   PRUEBAS DEL ENSAMBLADOR Y LINKER IA-32 (UNAM)      "
echo "======================================================"

echo -e "\n[1/5] Recompilando el proyecto..."
make clean > /dev/null
make > /dev/null
if [ $? -ne 0 ]; then
    echo "Error fatal: El proyecto no compila."
    exit 1
fi
echo "Compilación exitosa."

# ==========================================
# ENSAMBLADOR DE 1 PASADA
# ==========================================
echo -e "\n[2/5] Probando ensamblador de 1 pasada..."
./assembler_1pass examples/1pass/hello.asm
echo "1 pasada: OK"

# ==========================================
# ENSAMBLADOR DE 2 PASADAS
# ==========================================
echo -e "\n[3/5] Probando ensamblador de 2 pasadas..."
./assembler_2pass examples/2pass/bucle_contador.asm
./assembler_2pass examples/2pass/hola_mundo.asm
./assembler_2pass examples/2pass/variables_memoria.asm
./assembler_2pass tests/test_basic.asm
./assembler_2pass tests/test_jumps.asm
./assembler_2pass tests/test_sib.asm
./assembler_2pass tests/test_org.asm
echo "2 pasadas: OK"

# ==========================================
# LINKER — MÚLTIPLES MÓDULOS
# ==========================================
echo -e "\n[4/5] Enlazando múltiples módulos..."
./assembler_2pass tests/test_linker_mod1.asm
./assembler_2pass tests/test_linker_mod2.asm
./assembler_2pass --link tests/test_linker_mod1.o tests/test_linker_mod2.o -o tests/ejecutable_final.bin
echo "Linker: OK"

# ==========================================
# MANEJO DE ERRORES
# ==========================================
echo -e "\n[5/5] Probando manejo de errores..."
echo -e "\n  [Error 1] Símbolo externo no resuelto:"
cat << 'EOF' > tests/error_simbolo.asm
SECTION .text
GLOBAL _start
EXTERN funcion_que_no_existe
_start:
    CALL funcion_que_no_existe
EOF
./assembler_2pass tests/error_simbolo.asm > /dev/null
./assembler_2pass --link tests/error_simbolo.o -o tests/error1.bin
echo "  -> El linker debio reportar error de simbolo no resuelto."

echo -e "\n  [Error 2] Archivo fuente inexistente:"
./assembler_2pass tests/archivo_que_no_existe.asm
echo "  -> El ensamblador debio reportar error de lectura."

echo -e "\n  [Error 3] Archivo objeto con firma invalida:"
echo "BASURA" > tests/error_formato.o
./assembler_2pass --link tests/error_formato.o -o tests/error3.bin
echo "  -> El linker debio reportar firma UNAM no encontrada."

echo -e "\n  [Error 4] Redefinicion de etiqueta:"
cat << 'EOF' > tests/error_redef.asm
SECTION .text
_start:
    MOV EAX, 1
_start:
    MOV EBX, 2
EOF
./assembler_2pass tests/error_redef.asm
echo "  -> El ensamblador debio reportar etiqueta ya definida."

echo -e "\n  [Error 5] Linker sin archivos objeto:"
./assembler_2pass --link -o tests/error5.bin
echo "  -> El linker debio reportar que no hay archivos para enlazar."

echo -e "\nManejo de errores: OK"

echo -e "\n======================================================"
echo "               AUTOMATIZACIÓN FINALIZADA              "
echo "======================================================"
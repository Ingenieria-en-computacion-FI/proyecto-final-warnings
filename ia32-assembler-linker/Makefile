CC = gcc
CFLAGS = -Wall -Wextra -g

# Directorios separados
SRC_1PASS = src/1pass
SRC_2PASS = src/2pass
INC_1PASS = -I./include/1pass
INC_2PASS = -I./include/2pass
OBJ_DIR   = obj

# Fuentes de cada pasada
SRCS_1PASS = $(wildcard $(SRC_1PASS)/*.c)
SRCS_2PASS = $(wildcard $(SRC_2PASS)/*.c)

# Objetos — van a subcarpetas separadas para evitar colisiones
OBJS_1PASS = $(patsubst $(SRC_1PASS)/%.c, $(OBJ_DIR)/1pass/%.o, $(SRCS_1PASS))
OBJS_2PASS = $(patsubst $(SRC_2PASS)/%.c, $(OBJ_DIR)/2pass/%.o, $(SRCS_2PASS))

TARGET1 = assembler_1pass
TARGET2 = assembler_2pass

all: $(TARGET1) $(TARGET2)

# Enlazado
$(TARGET1): $(OBJS_1PASS)
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET2): $(OBJS_2PASS)
	$(CC) $(CFLAGS) -o $@ $^

# Compilación 1pass
$(OBJ_DIR)/1pass/%.o: $(SRC_1PASS)/%.c
	@mkdir -p $(OBJ_DIR)/1pass
	$(CC) $(CFLAGS) $(INC_1PASS) -c $< -o $@

# Compilación 2pass
$(OBJ_DIR)/2pass/%.o: $(SRC_2PASS)/%.c
	@mkdir -p $(OBJ_DIR)/2pass
	$(CC) $(CFLAGS) $(INC_2PASS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET1) $(TARGET2) tests/*.o tests/*.bin

.PHONY: all clean
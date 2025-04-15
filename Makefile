CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99 -Iinclude
SRC_DIR = src
INCLUDE_DIR = include
SRC = $(SRC_DIR)/main.c $(SRC_DIR)/core/gc.c $(SRC_DIR)/tracer/gc_tracer.c
OBJ = $(SRC:.c=.o)
TARGET = heapster

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
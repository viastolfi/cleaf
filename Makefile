BUILD = build
SRC = src

CS = \
	$(SRC)/cleaf.c \
	$(SRC)/frontend/ast.c \

OBJ = \
	$(BUILD)/cleaf.o \
	$(BUILD)/frontend/ast.o \

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

.PHONY: all clean

all: $(BUILD)/cleaf

$(BUILD)/cleaf: $(OBJ)
	$(CC) -o $@ $^

$(BUILD)/%.o: $(SRC)/%.c
	@mkdir -p $(BUILD)
	@mkdir -p $(BUILD)/frontend
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD)


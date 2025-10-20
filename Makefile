BUILD = build
SRC = src

CS = \
	$(SRC)/cleaf.c \

OBJ = \
	$(BUILD)/cleaf.o \

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

.PHONY: all clean

all: $(BUILD)/cleaf

$(BUILD)/cleaf: $(OBJ)
	$(CC) -o $@ $^

$(BUILD)/%.o: $(SRC)/%.c
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD)


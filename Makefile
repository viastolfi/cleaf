BUILD = build
SRC = src
TEST = test

CS = \
        $(SRC)/cleaf.c \
        $(SRC)/frontend/ast.c \
        $(SRC)/frontend/error.c \

OBJ = \
        $(BUILD)/cleaf.o \
        $(BUILD)/frontend/ast.o \
        $(BUILD)/frontend/error.o \

CC = gcc
CFLAGS = -Wall -Wextra -g

.PHONY: all clean test

all: $(BUILD)/cleaf

$(BUILD)/cleaf: $(OBJ)
	$(CC) -o $@ $^ -lm
	@$(BUILD)/cleaf test.clf 2> cleaf.log

$(BUILD)/%.o: $(SRC)/%.c
	@mkdir -p $(BUILD)
	@mkdir -p $(BUILD)/frontend
	$(CC) $(CFLAGS) -c $< -o $@

TEST_SRC = $(TEST)/ast_test.c
TEST_BIN = $(BUILD)/ast_test

test: $(TEST_BIN)
	@echo "Running AST tests..."
	@$(TEST_BIN) 2> test.log

$(TEST_BIN): $(TEST_SRC) $(SRC)/frontend/ast.c $(SRC)/frontend/error.c
	@mkdir -p $(BUILD)
	@$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(BUILD)


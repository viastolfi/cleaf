BUILD = build
SRC = src
TEST = test

CS = \
        $(SRC)/cleaf.c \
        $(SRC)/frontend/ast.c \
        $(SRC)/frontend/error.c \
				$(SRC)/frontend/semantic.c \

OBJ = \
        $(BUILD)/cleaf.o \
        $(BUILD)/frontend/ast.o \
        $(BUILD)/frontend/error.o \
				$(BUILD)/frontend/semantic.o \

CC = gcc
CFLAGS = -Wall -Wextra -g

.PRECIOUS: build/cleaf
.PHONY: all clean test asan-test valgrind-test

all: $(BUILD)/cleaf

$(BUILD)/cleaf: $(OBJ)
	$(CC) -o $@ $^ -lm
	@$(BUILD)/cleaf test.clf 

$(BUILD)/%.o: $(SRC)/%.c
	@mkdir -p $(BUILD)
	@mkdir -p $(BUILD)/frontend
	$(CC) $(CFLAGS) -c $< -o $@

AST_TEST_SRC = $(TEST)/ast_test.c
AST_TEST_BIN = $(BUILD)/ast_test

SEM_TEST_SRC = $(TEST)/semantic_test.c
SEM_TEST_BIN = $(BUILD)/semantic_test

test: $(AST_TEST_BIN) $(SEM_TEST_BIN)
	@echo "Running tests..."
	@$(AST_TEST_BIN) 2> test.log
	@$(SEM_TEST_BIN) 2> test.log

$(AST_TEST_BIN): $(AST_TEST_SRC) $(SRC)/frontend/ast.c $(SRC)/frontend/error.c
	@mkdir -p $(BUILD)
	@$(CC) $(CFLAGS) $^ -o $@ -lm

$(SEM_TEST_BIN): $(SEM_TEST_SRC) $(SRC)/frontend/ast.c $(SRC)/frontend/error.c $(SRC)/frontend/semantic.c
	@mkdir -p $(BUILD)
	@$(CC) $(CFLAGS) $^ -o $@ -lm

asan-test:
	CFLAGS="-fsanitize=address,undefined -g -O1" make test

valgrind-test:
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./build/cleaf test.clf


clean:
	rm -rf $(BUILD)


BUILD = build
SRC = src
TEST = test

CS = \
        $(SRC)/cleaf.c \
        $(SRC)/frontend/ast.c \
        $(SRC)/thirdparty/error.c \
				$(SRC)/frontend/semantic.c \
				$(SRC)/middleend/hir.c \

OBJ = \
        $(BUILD)/cleaf.o \
        $(BUILD)/frontend/ast.o \
        $(BUILD)/thirdparty/error.o \
				$(BUILD)/frontend/semantic.o \
				$(BUILD)/middleend/hir.o \

CC = gcc
CFLAGS = -Wall -Wextra -g

.PRECIOUS: build/cleaf
.PHONY: all clean test ast-test semantic-test asan-test valgrind-test hir-test

all: $(BUILD)/cleaf

$(BUILD)/cleaf: $(OBJ)
	$(CC) -o $@ $^ -lm
	@$(BUILD)/cleaf test.clf 

$(BUILD)/%.o: $(SRC)/%.c
	@mkdir -p $(BUILD)
	@mkdir -p $(BUILD)/frontend
	@mkdir -p $(BUILD)/thirdparty
	@mkdir -p $(BUILD)/middleend
	$(CC) $(CFLAGS) -c $< -o $@

AST_TEST_SRC = $(TEST)/ast_test.c
AST_TEST_BIN = $(BUILD)/ast_test

SEM_TEST_SRC = $(TEST)/semantic_test.c
SEM_TEST_BIN = $(BUILD)/semantic_test

HIR_TEST_SRC = $(TEST)/hir_test.c
HIR_TEST_BIN = $(BUILD)/hir_test

test: $(AST_TEST_BIN) $(SEM_TEST_BIN)
	@echo "Running tests..."
	@$(AST_TEST_BIN) 2> test.log
	@$(SEM_TEST_BIN) 2> test.log
	@$(HIR_TEST_BIN) 2> test.log

ast-test: $(AST_TEST_BIN)
	@echo "Running AST tests..."
	@$(AST_TEST_BIN) 2> test.log

semantic-test: $(SEM_TEST_BIN)
	@echo "Running semantic tests..."
	@$(SEM_TEST_BIN) 2> test.log

hir-test: $(HIR_TEST_BIN)
	@echo "Running hir tests..."
	@$(HIR_TEST_BIN) 2> test.log

$(AST_TEST_BIN): $(AST_TEST_SRC) $(SRC)/frontend/ast.c $(SRC)/thirdparty/error.c
	@mkdir -p $(BUILD)
	@$(CC) $(CFLAGS) $^ -o $@ -lm

$(SEM_TEST_BIN): $(SEM_TEST_SRC) $(SRC)/frontend/ast.c $(SRC)/thirdparty/error.c $(SRC)/frontend/semantic.c
	@mkdir -p $(BUILD)
	@$(CC) $(CFLAGS) $^ -o $@ -lm

$(HIR_TEST_BIN): $(HIR_TEST_SRC) $(SRC)/frontend/ast.c $(SRC)/thirdparty/error.c $(SRC)/middleend/hir.c
	@mkdir -p $(BUILD)
	@$(CC) $(CFLAGS) $^ -o $@ -lm

asan-test:
	CFLAGS="-fsanitize=address,undefined -g -O1" make test

valgrind-test:
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./build/cleaf test.clf


clean:
	rm -rf $(BUILD)


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
.PHONY: all clean test ast-test semantic-test asan-test valgrind-test

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

ast-test: $(AST_TEST_BIN)
	@echo "Running AST tests..."
	@$(AST_TEST_BIN) 2> test.log

semantic-test: $(SEM_TEST_BIN)
	@echo "Running semantic tests..."
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
	@echo "Testing memory safety with valgrind..."
	@echo "=== Testing normal execution ==="
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./build/cleaf test/valgrind_case/full.clf
	@echo "=== Testing lexer errors ==="
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./build/cleaf test/valgrind_case/lexer_error.clf
	@echo "=== Testing parser errors ==="
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./build/cleaf test/valgrind_case/parser_missing_semicolon.clf
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./build/cleaf test/valgrind_case/parser_unmatched_brace.clf
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./build/cleaf test/valgrind_case/parser_missing_paren.clf
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./build/cleaf test/valgrind_case/parser_incomplete_function.clf
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./build/cleaf test/valgrind_case/parser_missing_function_name.clf
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./build/cleaf test/valgrind_case/parser_missing_var_name.clf
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./build/cleaf test/valgrind_case/parser_for_missing_increment.clf
	@echo "=== Testing semantic errors ==="
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./build/cleaf test/valgrind_case/semantic_function_reserved.clf
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./build/cleaf test/valgrind_case/semantic_undefined_vars.clf
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./build/cleaf test/valgrind_case/semantic_type_mismatch.clf
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./build/cleaf test/valgrind_case/semantic_var_redefinition.clf
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./build/cleaf test/valgrind_case/semantic_undefined_function.clf
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./build/cleaf test/valgrind_case/semantic_function_duplicate.clf
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./build/cleaf test/valgrind_case/semantic_function_args_error.clf
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./build/cleaf test/valgrind_case/semantic_return_type_error.clf
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./build/cleaf test/valgrind_case/semantic_unary_type_error.clf
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./build/cleaf test/valgrind_case/semantic_control_flow_errors.clf
	@echo "=== Testing combined errors ==="
	valgrind --error-exitcode=1 --leak-check=full --show-leak-kinds=all ./build/cleaf test/valgrind_case/combined_multiple_errors.clf
	@echo "=== All valgrind tests passed ==="

clean:
	rm -rf $(BUILD)


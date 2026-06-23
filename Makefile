BUILD = build
SRC = src
TEST = test

CS = \
        $(SRC)/cleaf.c \
        $(SRC)/frontend/ast.c \
        $(SRC)/thirdparty/error.c \
				$(SRC)/frontend/semantic.c \
				$(SRC)/middleend/hir.c \
				$(SRC)/frontend/ast_printer.c \
				$(SRC)/backend/x86_64.c \
				$(SRC)/backend/codegen.c \
				$(SRC)/compiler/definition/compiler_definition.c \
				$(SRC)/compiler/setup/compiler_setup.c \
				$(SRC)/compiler/build/file_scanner.c \

OBJ = \
        $(BUILD)/cleaf.o \
        $(BUILD)/frontend/ast.o \
        $(BUILD)/thirdparty/error.o \
				$(BUILD)/frontend/semantic.o \
				$(BUILD)/middleend/hir.o \
				$(BUILD)/frontend/ast_printer.o \
				$(BUILD)/backend/x86_64.o \
				$(BUILD)/backend/codegen.o \
				$(BUILD)/compiler/definition/compiler_definition.o \
				$(BUILD)/compiler/setup/compiler_setup.o \
				$(BUILD)/compiler/build/file_scanner.o \

CC = gcc
CFLAGS = -Wall -Wextra -g -Isrc
VALGRIND = valgrind --error-exitcode=42 --leak-check=full --show-leak-kinds=all

.PRECIOUS: build/cleaf
.PHONY: all clean test ast-test semantic-test asan-test valgrind-test hir-test codegen-test

all: $(BUILD)/cleaf

$(BUILD)/cleaf: $(OBJ)
	$(CC) -o $@ $^ -lm
	@$(BUILD)/cleaf test.clf 

$(BUILD)/%.o: $(SRC)/%.c
	@mkdir -p $(BUILD)
	@mkdir -p $(BUILD)/frontend
	@mkdir -p $(BUILD)/thirdparty
	@mkdir -p $(BUILD)/middleend
	@mkdir -p $(BUILD)/backend
	@mkdir -p $(BUILD)/compiler/definition
	@mkdir -p $(BUILD)/compiler/setup
	@mkdir -p $(BUILD)/compiler/build
	$(CC) $(CFLAGS) -c $< -o $@

AST_TEST_SRC = $(TEST)/ast_test.c
AST_TEST_BIN = $(BUILD)/ast_test

SEM_TEST_SRC = $(TEST)/semantic_test.c
SEM_TEST_BIN = $(BUILD)/semantic_test

HIR_TEST_SRC = $(TEST)/hir_test.c
HIR_TEST_BIN = $(BUILD)/hir_test

CODEGEN_TEST_SRC = $(TEST)/codegen_test.c
CODEGEN_TEST_BIN = $(BUILD)/codegen_test

test: $(AST_TEST_BIN) $(SEM_TEST_BIN) $(HIR_TEST_BIN) $(CODEGEN_TEST_BIN)
	@echo "Running tests..."
	@$(AST_TEST_BIN)
	@$(SEM_TEST_BIN)
	@$(HIR_TEST_BIN)
	@$(CODEGEN_TEST_BIN)

ast-test: $(AST_TEST_BIN)
	@echo "Running AST tests..."
	@$(AST_TEST_BIN) 2> test.log

semantic-test: $(SEM_TEST_BIN)
	@echo "Running semantic tests..."
	@$(SEM_TEST_BIN) 2> test.log

hir-test: $(HIR_TEST_BIN)
	@echo "Running hir tests..."
	@$(HIR_TEST_BIN) 2> test.log

codegen-test: $(CODEGEN_TEST_BIN)
	@echo "Running codegen tests..."
	@$(CODEGEN_TEST_BIN) 2> test.log

$(AST_TEST_BIN): $(AST_TEST_SRC) $(SRC)/frontend/ast.c $(SRC)/thirdparty/error.c
	@mkdir -p $(BUILD)
	@$(CC) $(CFLAGS) $^ -o $@ -lm

$(SEM_TEST_BIN): $(SEM_TEST_SRC) $(SRC)/frontend/ast.c $(SRC)/thirdparty/error.c $(SRC)/frontend/semantic.c
	@mkdir -p $(BUILD)
	@$(CC) $(CFLAGS) $^ -o $@ -lm

$(HIR_TEST_BIN): $(HIR_TEST_SRC) $(SRC)/frontend/ast.c $(SRC)/thirdparty/error.c $(SRC)/frontend/semantic.c $(SRC)/middleend/hir.c
	@mkdir -p $(BUILD)
	@$(CC) $(CFLAGS) $^ -o $@ -lm

$(CODEGEN_TEST_BIN): $(CODEGEN_TEST_SRC) $(SRC)/frontend/ast.c $(SRC)/thirdparty/error.c $(SRC)/frontend/semantic.c $(SRC)/middleend/hir.c $(SRC)/backend/x86_64.c $(SRC)/backend/codegen.c
	@mkdir -p $(BUILD)
	@$(CC) $(CFLAGS) $^ -o $@ -lm

asan-test:
	CFLAGS="-fsanitize=address,undefined -g -O1" make test

valgrind-test:
	@echo "Testing memory safety with valgrind..."
	@echo "=== Testing normal execution ==="
	$(VALGRIND) ./build/cleaf test/valgrind_case/full.clf
	@echo "=== Testing lexer errors ==="
	$(VALGRIND) ./build/cleaf test/valgrind_case/lexer_error.clf; [ $$? -ne 42 ]
	@echo "=== Testing parser errors ==="
	$(VALGRIND) ./build/cleaf test/valgrind_case/parser_missing_semicolon.clf; [ $$? -ne 42 ]
	$(VALGRIND) ./build/cleaf test/valgrind_case/parser_unmatched_brace.clf; [ $$? -ne 42 ]
	$(VALGRIND) ./build/cleaf test/valgrind_case/parser_missing_paren.clf; [ $$? -ne 42 ]
	$(VALGRIND) ./build/cleaf test/valgrind_case/parser_incomplete_function.clf; [ $$? -ne 42 ]
	$(VALGRIND) ./build/cleaf test/valgrind_case/parser_missing_function_name.clf; [ $$? -ne 42 ]
	$(VALGRIND) ./build/cleaf test/valgrind_case/parser_missing_var_name.clf; [ $$? -ne 42 ]
	$(VALGRIND) ./build/cleaf test/valgrind_case/parser_for_missing_increment.clf; [ $$? -ne 42 ]
	@echo "=== Testing semantic errors ==="
	$(VALGRIND) ./build/cleaf test/valgrind_case/semantic_function_reserved.clf; [ $$? -ne 42 ]
	$(VALGRIND) ./build/cleaf test/valgrind_case/semantic_undefined_vars.clf; [ $$? -ne 42 ]
	$(VALGRIND) ./build/cleaf test/valgrind_case/semantic_type_mismatch.clf; [ $$? -ne 42 ]
	$(VALGRIND) ./build/cleaf test/valgrind_case/semantic_var_redefinition.clf; [ $$? -ne 42 ]
	$(VALGRIND) ./build/cleaf test/valgrind_case/semantic_undefined_function.clf; [ $$? -ne 42 ]
	$(VALGRIND) ./build/cleaf test/valgrind_case/semantic_function_duplicate.clf; [ $$? -ne 42 ]
	$(VALGRIND) ./build/cleaf test/valgrind_case/semantic_function_args_error.clf; [ $$? -ne 42 ]
	$(VALGRIND) ./build/cleaf test/valgrind_case/semantic_return_type_error.clf; [ $$? -ne 42 ]
	$(VALGRIND) ./build/cleaf test/valgrind_case/semantic_unary_type_error.clf; [ $$? -ne 42 ]
	$(VALGRIND) ./build/cleaf test/valgrind_case/semantic_control_flow_errors.clf; [ $$? -ne 42 ]
	@echo "=== Testing combined errors ==="
	$(VALGRIND) ./build/cleaf test/valgrind_case/combined_multiple_errors.clf; [ $$? -ne 42 ]
	@echo "=== All valgrind tests passed ==="

clean:
	rm -rf $(BUILD)


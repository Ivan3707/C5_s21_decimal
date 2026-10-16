CC = gcc
AR = ar
ARFLAGS = rcs
UNAME_S := $(shell uname -s)

CFLAGS = -std=c11 -Wall -Wextra -Werror
TEST_CFLAGS = $(CFLAGS) -I.

ifeq ($(UNAME_S),Linux)
TEST_CFLAGS += -Wno-stringop-truncation -Wno-stringop-overflow
endif

COVERAGE_FLAGS = -fprofile-arcs -ftest-coverage

PKG_CONFIG ?= pkg-config
CHECK_CFLAGS = $(shell $(PKG_CONFIG) --cflags check 2>/dev/null)
CHECK_LIBS = $(shell $(PKG_CONFIG) --libs check 2>/dev/null)

ifeq ($(strip $(CHECK_LIBS)),)
ifeq ($(OS),Windows_NT)
CHECK_CFLAGS = -I"C:/msys64/ucrt64/include"
CHECK_LIBS = -L"C:/msys64/ucrt64/lib" -lcheck
else
ifeq ($(UNAME_S),Linux)
CHECK_LIBS = -lcheck -lsubunit -lm -lpthread -lrt
endif
ifeq ($(UNAME_S),Darwin)
CHECK_LIBS = -lcheck -lm -lpthread
endif
endif
endif

TARGET = s21_decimal.a

SRC = s21_add.c s21_sub.c s21_div.c s21_other.c s21_helpers.c s21_compare.c s21_mul.c s21_convert.c
OBJ = $(SRC:.c=.o)

TEST_SRC = tests/test_add.c tests/test_sub.c tests/test_runner.c tests/test_div.c tests/test_other.c tests/test_helpers.c tests/test_compare.c tests/test_mul.c tests/test_convert.c

CLANG_FORMAT ?= clang-format
CPPCHECK ?= cppcheck
FORMAT_CONFIG = ../materials/linters/.clang-format
FORMAT_FILES = $(SRC) $(TEST_SRC) s21_decimal.h s21_decimal_internal.h

TEST_BIN = test.exe
HELPERS_TEST_BIN = test_helpers.exe
ADD_TEST_BIN = test_add.exe

.PHONY: all clean test test_helpers test_add gcov_report rebuild format \
	format_check cppcheck lint leaks valgrind memcheck

all: $(TARGET)

$(TARGET): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^
	rm -f $(OBJ)

%.o: %.c s21_decimal.h s21_decimal_internal.h
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_BIN): $(TEST_SRC) $(TARGET) s21_decimal.h
	$(CC) $(TEST_CFLAGS) $(CHECK_CFLAGS) $(TEST_SRC) $(TARGET) $(CHECK_LIBS) -o $(TEST_BIN)

test: $(TEST_BIN)
	./$(TEST_BIN)

$(HELPERS_TEST_BIN): tests/test_helpers.c s21_helpers.c s21_decimal.h
	$(CC) $(CFLAGS) $(CHECK_CFLAGS) -DS21_HELPERS_TEST_MAIN tests/test_helpers.c s21_helpers.c $(CHECK_LIBS) -o $@

test_helpers: $(HELPERS_TEST_BIN)
	./$(HELPERS_TEST_BIN)

$(ADD_TEST_BIN): tests/test_add.c s21_add.c s21_helpers.c s21_decimal.h
	$(CC) $(CFLAGS) $(CHECK_CFLAGS) -DS21_ADD_TEST_MAIN tests/test_add.c s21_add.c s21_helpers.c $(CHECK_LIBS) -o $@

test_add: $(ADD_TEST_BIN)
	./$(ADD_TEST_BIN)

format:
	@test -f $(FORMAT_CONFIG) || (echo "Не найден $(FORMAT_CONFIG)" && exit 1)
	@command -v $(CLANG_FORMAT) >/dev/null || (echo "clang-format не установлен" && exit 1)
	$(CLANG_FORMAT) -i --style=file:$(FORMAT_CONFIG) $(FORMAT_FILES)

format_check:
	@test -f $(FORMAT_CONFIG) || (echo "Не найден $(FORMAT_CONFIG)" && exit 1)
	@command -v $(CLANG_FORMAT) >/dev/null || (echo "clang-format не установлен" && exit 1)
	$(CLANG_FORMAT) --dry-run --Werror --style=file:$(FORMAT_CONFIG) $(FORMAT_FILES)

cppcheck:
	@command -v $(CPPCHECK) >/dev/null || (echo "cppcheck не установлен" && exit 1)
	$(CPPCHECK) --enable=warning,performance,portability --std=c11 \
		--error-exitcode=1 --suppress=missingIncludeSystem --quiet $(SRC)

lint: format_check cppcheck

leaks: $(TEST_BIN)
	@command -v leaks >/dev/null || (echo "leaks доступен только на macOS" && exit 1)
	CK_FORK=no leaks -atExit -- ./$(TEST_BIN)

valgrind: $(TEST_BIN)
	@command -v valgrind >/dev/null || (echo "valgrind не установлен" && exit 1)
	CK_FORK=no valgrind --tool=memcheck --leak-check=full \
		--show-leak-kinds=all --errors-for-leak-kinds=all \
		--error-exitcode=1 ./$(TEST_BIN)

ifeq ($(UNAME_S),Darwin)
memcheck: leaks
else
memcheck: valgrind
endif

gcov_report:
	$(CC) $(CFLAGS) $(COVERAGE_FLAGS) -c $(SRC)
	$(AR) $(ARFLAGS) $(TARGET) $(OBJ)
	$(CC) $(TEST_CFLAGS) $(COVERAGE_FLAGS) $(CHECK_CFLAGS) $(TEST_SRC) $(TARGET) $(CHECK_LIBS) -o $(TEST_BIN)
	./$(TEST_BIN)
	gcovr --root . --filter 's21_.*\.c' --html --html-self-contained --output coverage.html
	gcovr --root . --filter 's21_.*\.c' --print-summary
	rm -f $(TARGET) $(OBJ) $(TEST_BIN)
	rm -f *.gcda *.gcno *.gcov

clean:
	rm -f $(ADD_TEST_BIN)
	rm -f $(HELPERS_TEST_BIN)
	rm -f $(TARGET) $(OBJ) $(TEST_BIN)
	rm -f *.gcda *.gcno *.gcov
	rm -f coverage.html coverage.css coverage.js coverage.functions.html
	rm -f coverage.*.html

rebuild: clean all

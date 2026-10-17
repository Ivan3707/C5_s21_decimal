CC = gcc
AR = ar
ARFLAGS = rcs

UNAME_S := $(shell uname -s)

TARGET = s21_decimal.a
TEST_BIN = test.exe
GCOV_BIN = gcov_test.exe

HEADERS = s21_decimal.h s21_decimal_internal.h
SRC = \
	s21_add.c \
	s21_sub.c \
	s21_div.c \
	s21_other.c \
	s21_helpers.c \
	s21_compare.c \
	s21_mul.c \
	s21_convert.c
OBJ = $(SRC:.c=.o)

TEST_SRC = \
	tests/test_runner.c \
	tests/test_add.c \
	tests/test_sub.c \
	tests/test_div.c \
	tests/test_other.c \
	tests/test_helpers.c \
	tests/test_compare.c \
	tests/test_mul.c \
	tests/test_convert.c

CFLAGS = -std=c11 -Wall -Wextra -Werror
TEST_CFLAGS = $(CFLAGS) -I.
COVERAGE_FLAGS = -fprofile-arcs -ftest-coverage

ifeq ($(UNAME_S),Linux)
TEST_CFLAGS += -Wno-stringop-truncation -Wno-stringop-overflow
endif

PKG_CONFIG ?= pkg-config
CHECK_CFLAGS := $(shell $(PKG_CONFIG) --cflags check 2>/dev/null)
CHECK_LIBS := $(shell $(PKG_CONFIG) --libs check 2>/dev/null)

ifeq ($(strip $(CHECK_LIBS)),)
ifeq ($(OS),Windows_NT)
CHECK_CFLAGS = -I"C:/msys64/ucrt64/include"
CHECK_LIBS = -L"C:/msys64/ucrt64/lib" -lcheck
else ifeq ($(UNAME_S),Linux)
CHECK_LIBS = -lcheck -lsubunit -lm -lpthread -lrt
else ifeq ($(UNAME_S),Darwin)
CHECK_LIBS = -lcheck -lm -lpthread
endif
endif

CLANG_FORMAT ?= clang-format
CPPCHECK ?= cppcheck
GCOVR ?= gcovr
LEAKS ?= leaks
VALGRIND ?= valgrind

FORMAT_CONFIG = ../materials/linters/.clang-format
FORMAT_FILES = $(SRC) $(TEST_SRC) $(HEADERS)
FORMAT_FLAGS = --style=file:$(FORMAT_CONFIG)
CPPCHECK_FLAGS = --enable=warning,performance,portability --std=c11 \
	--error-exitcode=1 --suppress=missingIncludeSystem --quiet
GCOVR_FLAGS = --root . --filter 's21_.*\.c'
VALGRIND_FLAGS = --tool=memcheck --leak-check=full \
	--show-leak-kinds=all --errors-for-leak-kinds=all --error-exitcode=1

.PHONY: all clean rebuild test gcov_report format format_check cppcheck lint \
	leaks valgrind memcheck

all: $(TARGET)

$(TARGET): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_BIN): $(TEST_SRC) $(TARGET) $(HEADERS)
	$(CC) $(TEST_CFLAGS) $(CHECK_CFLAGS) $(TEST_SRC) $(TARGET) \
		$(CHECK_LIBS) -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

format:
	@test -f $(FORMAT_CONFIG) || (echo "Не найден $(FORMAT_CONFIG)" && exit 1)
	@command -v $(CLANG_FORMAT) >/dev/null || \
		(echo "clang-format не установлен" && exit 1)
	$(CLANG_FORMAT) -i $(FORMAT_FLAGS) $(FORMAT_FILES)

format_check:
	@test -f $(FORMAT_CONFIG) || (echo "Не найден $(FORMAT_CONFIG)" && exit 1)
	@command -v $(CLANG_FORMAT) >/dev/null || \
		(echo "clang-format не установлен" && exit 1)
	$(CLANG_FORMAT) --dry-run --Werror $(FORMAT_FLAGS) $(FORMAT_FILES)

cppcheck:
	@command -v $(CPPCHECK) >/dev/null || \
		(echo "cppcheck не установлен" && exit 1)
	$(CPPCHECK) $(CPPCHECK_FLAGS) $(SRC)

lint: format_check cppcheck

leaks: $(TEST_BIN)
	@command -v $(LEAKS) >/dev/null || \
		(echo "leaks доступен только на macOS" && exit 1)
	CK_FORK=no $(LEAKS) -atExit -- ./$(TEST_BIN)

valgrind: $(TEST_BIN)
	@command -v $(VALGRIND) >/dev/null || \
		(echo "valgrind не установлен" && exit 1)
	CK_FORK=no $(VALGRIND) $(VALGRIND_FLAGS) ./$(TEST_BIN)

ifeq ($(UNAME_S),Darwin)
memcheck: leaks
else
memcheck: valgrind
endif

gcov_report:
	$(RM) $(GCOV_BIN) *.gcda *.gcno *.gcov
	$(CC) $(TEST_CFLAGS) $(COVERAGE_FLAGS) $(CHECK_CFLAGS) $(SRC) \
		$(TEST_SRC) $(CHECK_LIBS) -o $(GCOV_BIN)
	./$(GCOV_BIN)
	$(GCOVR) $(GCOVR_FLAGS) --html --html-self-contained \
		--output coverage.html
	$(GCOVR) $(GCOVR_FLAGS) --print-summary
	$(RM) $(GCOV_BIN) *.gcda *.gcno *.gcov

clean:
	$(RM) $(TARGET) $(OBJ) $(TEST_BIN) $(GCOV_BIN)
	$(RM) *.gcda *.gcno *.gcov
	$(RM) coverage.html coverage.css coverage.js coverage.functions.html
	$(RM) coverage.*.html

rebuild: clean all

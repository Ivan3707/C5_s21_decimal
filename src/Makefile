CC = gcc
AR = ar
ARFLAGS = rcs

CFLAGS = -std=c11 -Wall -Wextra -Werror
TEST_CFLAGS = -std=c11 -Wall -Wextra -Wno-memset-transposed-args \
              -Wno-stringop-truncation -Wno-stringop-overflow -I.

COVERAGE_FLAGS = -fprofile-arcs -ftest-coverage

PKG_CONFIG ?= pkg-config
CHECK_CFLAGS = $(shell $(PKG_CONFIG) --cflags check 2>/dev/null)
CHECK_LIBS = $(shell $(PKG_CONFIG) --libs check 2>/dev/null)

ifeq ($(strip $(CHECK_LIBS)),)
ifeq ($(OS),Windows_NT)
CHECK_CFLAGS = -I"C:/msys64/ucrt64/include"
CHECK_LIBS = -L"C:/msys64/ucrt64/lib" -lcheck
else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
CHECK_LIBS = -lcheck -lsubunit -lm -lpthread -lrt
endif
ifeq ($(UNAME_S),Darwin)
CHECK_LIBS = -lcheck -lm -lpthread
endif
endif
endif

TARGET = s21_decimal.a

SRC = s21_div.c s21_other.c s21_helpers.c s21_compare.c s21_mul.c s21_convert.c
OBJ = $(SRC:.c=.o)

TEST_SRC = tests/test_runner.c tests/test_div.c tests/test_other.c tests/test_helpers.c tests/test_compare.c tests/test_mul.c tests/test_convert.c
TEST_BIN = test.exe
HELPERS_TEST_BIN = test_helpers.exe

.PHONY: all clean test test_helpers gcov_report rebuild

all: $(TARGET)

$(TARGET): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^
	rm -f $(OBJ)

%.o: %.c s21_decimal.h
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_BIN): $(TEST_SRC) $(TARGET) s21_decimal.h
	$(CC) $(TEST_CFLAGS) $(CHECK_CFLAGS) $(TEST_SRC) $(TARGET) $(CHECK_LIBS) -o $(TEST_BIN)

test: $(TEST_BIN)
	./$(TEST_BIN)

$(HELPERS_TEST_BIN): tests/test_helpers.c s21_helpers.c s21_decimal.h
	$(CC) $(CFLAGS) $(CHECK_CFLAGS) -DS21_HELPERS_TEST_MAIN tests/test_helpers.c s21_helpers.c $(CHECK_LIBS) -o $@

test_helpers: $(HELPERS_TEST_BIN)
	./$(HELPERS_TEST_BIN)

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
	rm -f $(HELPERS_TEST_BIN)
	rm -f $(TARGET) $(OBJ) $(TEST_BIN)
	rm -f *.gcda *.gcno *.gcov
	rm -f coverage.html coverage.css coverage.js coverage.functions.html
	rm -f coverage.*.html

rebuild: clean all

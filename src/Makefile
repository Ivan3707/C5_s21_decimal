.RECIPEPREFIX = >

CC = gcc
AR = ar
ARFLAGS = rcs

CFLAGS = -std=c11 -Wall -Wextra -Werror
TEST_CFLAGS = -std=c11 -Wall -Wextra -Wno-memset-transposed-args \
              -Wno-stringop-truncation -Wno-stringop-overflow -I.

COVERAGE_FLAGS = -fprofile-arcs -ftest-coverage

ifeq ($(OS),Windows_NT)
CHECK_INC = -I"C:/msys64/ucrt64/include"
CHECK_LIB = -L"C:/msys64/ucrt64/lib" -lcheck
else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
CHECK_INC =
CHECK_LIB = -lcheck -lsubunit -lm -lpthread -lrt
endif
ifeq ($(UNAME_S),Darwin)
CHECK_INC =
CHECK_LIB = -lcheck -lm -lpthread
endif
endif

TARGET = s21_decimal.a

SRC = s21_div.c
OBJ = $(SRC:.c=.o)

TEST_SRC = tests/test_runner.c tests/test_div.c
TEST_BIN = test.exe

.PHONY: all clean test s21_decimal.a gcov_report rebuild

all: $(TARGET)

$(TARGET): $(OBJ)
> $(AR) $(ARFLAGS) $@ $^

s21_decimal.a: $(TARGET)

%.o: %.c s21_decimal.h
> $(CC) $(CFLAGS) -c $< -o $@

$(TEST_BIN): $(TEST_SRC) $(TARGET) s21_decimal.h
> $(CC) $(TEST_CFLAGS) $(CHECK_INC) $(TEST_SRC) $(TARGET) $(CHECK_LIB) -o $(TEST_BIN)

test: $(TEST_BIN)
> ./$(TEST_BIN)

gcov_report:
> $(CC) $(CFLAGS) $(COVERAGE_FLAGS) -c s21_div.c -o s21_div.o
> $(AR) $(ARFLAGS) $(TARGET) s21_div.o
> $(CC) $(TEST_CFLAGS) $(COVERAGE_FLAGS) $(CHECK_INC) $(TEST_SRC) $(TARGET) $(CHECK_LIB) -o $(TEST_BIN)
> ./$(TEST_BIN)
> gcovr --root . --filter 's21_.*\.c' --html --html-self-contained --output coverage.html
> gcovr --root . --filter 's21_.*\.c' --print-summary
> rm -f $(TARGET) $(OBJ) $(TEST_BIN)
> rm -f *.gcda *.gcno *.gcov

clean:
> rm -f $(TARGET) $(OBJ) $(TEST_BIN)
> rm -f *.gcda *.gcno *.gcov
> rm -f coverage.html coverage.css coverage.js coverage.functions.html
> rm -f coverage.*.html

rebuild: clean all
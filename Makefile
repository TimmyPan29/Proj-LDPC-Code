# LDPC (1023, 781) simulator

CC      ?= cc
CFLAGS  ?= -O2 -std=c11 -Wall -Wextra
LDFLAGS ?= -lm

SRC_DIR  := src
TEST_DIR := tests
BIN      := ldpc_sim
SRCS     := $(wildcard $(SRC_DIR)/*.c)
OBJS     := $(SRCS:.c=.o)
LIB_OBJS := $(filter-out $(SRC_DIR)/main.o,$(OBJS))   # everything but main()
TEST_BIN := $(TEST_DIR)/selftest

.PHONY: all run test clean

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build and run with the default configuration (config/sim.txt).
run: $(BIN)
	./$(BIN) config/sim.txt

# Build and run the correctness self-test.
test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(TEST_DIR)/selftest.o $(LIB_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TEST_DIR)/selftest.o: $(TEST_DIR)/selftest.c
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

clean:
	rm -f $(OBJS) $(BIN) $(TEST_DIR)/*.o $(TEST_BIN)

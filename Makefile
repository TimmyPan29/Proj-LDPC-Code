# LDPC (1023, 781) simulator

CC      ?= cc
CFLAGS  ?= -O2 -std=c11 -Wall -Wextra
LDFLAGS ?= -lm

SRC_DIR := src
BIN     := ldpc_sim
SRCS    := $(wildcard $(SRC_DIR)/*.c)
OBJS    := $(SRCS:.c=.o)

.PHONY: all run clean

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build and run with the default configuration (config/sim.txt).
run: $(BIN)
	./$(BIN) config/sim.txt

clean:
	rm -f $(OBJS) $(BIN)

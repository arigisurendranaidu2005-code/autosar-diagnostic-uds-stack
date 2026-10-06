CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude
SRC = src/uds_protocol.c src/dtc_manager.c

all: uds_demo test

uds_demo: $(SRC) src/main.c
	$(CC) $(CFLAGS) $^ -o uds_stack_demo

test: $(SRC) tests/test_uds.c
	$(CC) $(CFLAGS) $^ -o tests/test_uds
	./tests/test_uds

clean:
	rm -f uds_stack_demo tests/test_uds *.exe tests/*.exe

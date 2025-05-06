CC = gcc
CFLAGS = -Ofast -march=native -g -fPIC -I$(INC_DIR)
LDFLAGS = -shared
INSTALL_DIR = install
LIB_DIR = $(INSTALL_DIR)/lib
BIN_DIR = $(INSTALL_DIR)/bin
INC_DIR = src  # Header file location

LIB_NAME = libthread
LIB_SO = $(LIB_DIR)/$(LIB_NAME).so

SRC = src/thread.c src/list_handler.c
OBJ = $(SRC:.c=.o)
OBJ_PREEMPT = src/thread_preempt.o src/list_handler.o
DEPS = src/thread.h  src/list_handler.h # Header file dependency

# List of tests
TESTS = tst/01-main tst/02-switch tst/03-equity tst/11-join tst/12-join-main tst/13-join-switch tst/31-switch-many \
        tst/61-mutex tst/21-create-many tst/32-switch-many-join \
        tst/62-mutex tst/81-deadlock tst/22-create-many-recursive \
        tst/33-switch-many-cascade tst/63-mutex-equity tst/23-create-many-once \
        tst/51-fibonacci tst/64-mutex-join tst/65-mutex-sum tst/parallel_sort
TESTWITHPREP = tst/66-semaphore tst/71-preemption
# Generate test targets in $(BIN_DIR)
TEST_TARGETS = $(patsubst tst/%, $(BIN_DIR)/%, $(TESTS))
TEST_TARGETS_PTHREAD = $(patsubst tst/%, $(BIN_DIR)/p%, $(TESTS))

.PHONY: all clean install pthread

# Default target: compile the library and all tests
all: $(LIB_SO) $(TEST_TARGETS) $(BIN_DIR)/71-preemption $(BIN_DIR)/66-semaphore

# Build shared library
$(LIB_SO): $(OBJ)
	@mkdir -p $(LIB_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# Compile source files into object files (recompile if thread.h changes)
src/%.o: src/%.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile test executables and place them in $(BIN_DIR) with custom library
$(BIN_DIR)/%: tst/%.c $(LIB_SO)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $< -L$(LIB_DIR) -lthread -Wl,-rpath,$(LIB_DIR)

# Compile test executables and place them in $(BIN_DIR) using pthread
$(BIN_DIR)/p%: tst/%.c $(LIB_SO)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $< -L$(LIB_DIR) -lthread  -DUSE_PTHREAD -Wl,-rpath,$(LIB_DIR)

src/thread_preempt.o: src/thread.c $(DEPS)
	$(CC) $(CFLAGS) -DPREEMPTION -c $< -o $@

$(LIB_DIR)/libthread_preempt.so: $(OBJ_PREEMPT)
	@mkdir -p $(LIB_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BIN_DIR)/71-preemption: tst/71-preemption.c $(LIB_DIR)/libthread_preempt.so
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -DPREEMPTION -o $@ $< -L$(LIB_DIR) -lthread_preempt -Wl,-rpath,$(LIB_DIR)
	valgrind --version

$(BIN_DIR)/66-semaphore: tst/66-semaphore.c $(LIB_DIR)/libthread_preempt.so
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -DPREEMPTION -o $@ $< -L$(LIB_DIR) -lthread_preempt -Wl,-rpath,$(LIB_DIR)



# Install the library and executables
install: all
#	rm -rf $(OBJ) $(BIN_DIR) $(LIB_DIR)
	@mkdir -p $(LIB_DIR) $(BIN_DIR)

# Install pthread executables
pthread: $(TEST_TARGETS_PTHREAD)
	@echo "Installed pthread test executables in $(BIN_DIR)."



# Run a specific test with Valgrind
valgrind/$(BIN_DIR)/%: $(BIN_DIR)/%
	@echo "Running Valgrind on $<..."
	@valgrind --leak-check=full --show-reachable=yes --track-origins=yes $<

# Run all tests with Valgrind
valgrind: $(patsubst $(BIN_DIR)/%, valgrind/$(BIN_DIR)/%,$(TEST_TARGETS))
	@echo "All tests completed under Valgrind."

# Clean up build files
clean:
	rm -rf $(OBJ) $(BIN_DIR) $(LIB_DIR)

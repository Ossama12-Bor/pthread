CC = gcc
CFLAGS = -Wall -Wextra -g -fPIC
LDFLAGS = -shared
VALGRIND_FLAGS = --leak-check=full --show-reachable=yes --track-origins=yes
INSTALL_DIR = install
LIB_DIR = $(INSTALL_DIR)/lib
BIN_DIR = $(INSTALL_DIR)/bin

LIB_NAME = libthread
LIB_SO = $(LIB_DIR)/$(LIB_NAME).so
LIB_A = $(LIB_DIR)/$(LIB_NAME).a

SRC = src/thread.c
OBJ = $(SRC:.c=.o)

TESTS = tst/01-main tst/02-switch tst/11-join tst/12-join-main tst/31-switch-many \
        tst/61-mutex tst/71-preemption tst/21-create-many tst/32-switch-many-join \
        tst/62-mutex tst/81-deadlock tst/22-create-many-recursive \
        tst/33-switch-many-cascade tst/63-mutex-equity tst/23-create-many-once \
        tst/51-fibonacci tst/64-mutex-join

TESTS_PTHREAD = $(addsuffix -pthread, $(TESTS))

.PHONY: all check valgrind pthreads graphs install clean

# Cible par défaut : compilation de la bibliothèque et des tests
all: $(LIB_SO) $(TESTS)

# Compilation de la bibliothèque partagée
$(LIB_SO): $(OBJ)
	@mkdir -p $(LIB_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

# Compilation de la bibliothèque statique
$(LIB_A): $(OBJ)
	@mkdir -p $(LIB_DIR)
	ar rcs $@ $^

# Compilation des fichiers objets
$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $< -o $@

# Compilation des tests
$(BIN_DIR)/%: tst/%.c $(LIB_SO)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $< -L$(LIB_DIR) -lthread

# Exécuter les tests
check: all
	for test in $(TESTS); do \
		$(BIN_DIR)/$$test; \
	done

# Exécuter les tests sous Valgrind
valgrind: all
	for test in $(TESTS); do \
		valgrind $(VALGRIND_FLAGS) $(BIN_DIR)/$$test; \
	done

# Compiler les tests avec pthreads
pthreads: clean
	$(MAKE) CFLAGS="$(CFLAGS) -DUSE_PTHREAD" LDFLAGS="-lpthread" all

# Générer les graphes
graphs: all
	@echo "Génération des graphes..."
	# Ajoutez ici la commande pour tracer les graphes

# Installation des fichiers compilés
install: all
	@mkdir -p $(LIB_DIR) $(BIN_DIR)
	cp $(LIB_SO) $(LIB_A) $(LIB_DIR)/
	f

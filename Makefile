BUILD_DIR = build

CC = gcc
CFLAGS = -Wall -Wextra
CFLAGS += -pedantic -Wpedantic

LDLIBS = -lncurses -lm

VPATH += src
SRC += commands.c
SRC += lex.c
SRC += view.c
SRC += main.c
SRC += cgetopt.c
SRC += frontend.c
SRC += keys.c
SRC += buffer.c
SRC += tools.c

OBJ := $(SRC:%.c=$(BUILD_DIR)/release-objs/%.o)
OBJ_DEBUG := $(SRC:%.c=$(BUILD_DIR)/debug-objs/%.o)

PREFIX ?= /usr

BINDIR ?= $(PREFIX)/bin/
HELP_DIR ?= $(PREFIX)/share/cano/help

ABS_HELP_DIR = $(shell realpath --canonicalize-missing $(HELP_DIR))

CFLAGS_main += -DHELP_DIR='"$(ABS_HELP_DIR)"'

all: cano
cano: $(BUILD_DIR)/cano
debug: $(BUILD_DIR)/debug

.PHONY: all cano debug

$(BUILD_DIR)/release-objs/%.o: %.c
	@ mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CFLAGS_$(notdir $(@:.o=))) -o $@ -c $< || exit 1

$(BUILD_DIR)/debug-objs/%.o: %.c
	@ mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CFLAGS_$(notdir $(@:.o=))) -o $@ -c $< || exit 1

$(BUILD_DIR)/cano: $(OBJ)
	@ mkdir -p $(dir $@)
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/debug: CFLAGS += -ggdb2
$(BUILD_DIR)/debug: $(OBJ_DEBUG)
	@ mkdir -p $(dir $@)
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS) $(LDLIBS)

clean:
	$(RM) $(OBJS) $(OBJ_DEBUG)

fclean:
	$(RM) -r $(BUILD_DIR)

.NOTPARALLEL: re
re: fclean all

.PHONY: clean fclean re

install: all
	@ install -Dv ./docs/help/* -t $(HELP_DIR)
	@ install -Dv ./build/cano -t $(BINDIR)

uninstall:
	@ $(RM) -r ~/$(HELP_DIR)
	@ $(RM) $(BINDIR)

.PHONY: install uninstall

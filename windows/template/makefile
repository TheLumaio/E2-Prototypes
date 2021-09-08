## Yoinked (and modified) from https://spin.atomicobject.com/2016/08/26/makefile-c-projects/

OUTPUT ?= OUT.EXE

BIN_DIR ?= ./bin
OBJ_DIR ?= ./obj
SRC_DIR ?= ./src

SRCS := $(shell find $(SRC_DIR) -name *.c)
OBJS := $(SRCS:%=$(OBJ_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

XNC_DIR ?= ../../

INC_DIRS := $(shell find $(SRC_DIR) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS)) -I$(XNC_DIR)/src

LDFLAGS := -pthread -L$(XNC_DIR)/bin -lxnc  -lraylib -lgdi32 -luser32 -lshell32 -lkernel32 -lwinmm -lws2_32

CFLAGS ?= $(INC_FLAGS) -std=c99 -ggdb

MKDIR_P	?= mkdir -p
CC = clang

all: pre_build $(OUTPUT)

pre_build:
	compiledb -n make

run: $(OUTPUT)
	cd $(BIN_DIR) && ./$(OUTPUT)

clean:
	$(RM) -r $(OBJ_DIR)

$(OUTPUT): $(OBJS)
	$(CC) $(OBJS) -o $(BIN_DIR)/$@ $(LDFLAGS)

$(OBJ_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

-include $(DEPS)

## Yoinked from https://spin.atomicobject.com/2016/08/26/makefile-c-projects/

TARGET_EXEC ?= MATRIX.EXE

BIN_DIR ?= ./bin
BUILD_DIR ?= ./obj
SRC_DIRS ?= ./src

FIND = find

SRCS := $(shell find $(SRC_DIRS) -name *.c)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

XNC_DIR ?= ../../

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS)) -I$(XNC_DIR)/src

LDFLAGS := -std=c99 -L$(XNC_DIR)/bin -lxnc  -lraylib -lgdi32 -luser32 -lshell32 -lkernel32 -lwinmm -lws2_32
# LDFLAGS := -std=c99 -L$(XNC_DIR)/bin -l:libxnc.a -lraylib -lssl -lcrypto -lm -lGL -lpthread -ldl -lrt -lX11

CPPFLAGS ?= $(INC_FLAGS) -ggdb

MKDIR_P ?= mkdir -p
CC = clang --target=x86_64-pc-windows-gnu

all: pre_build $(TARGET_EXEC)

pre_build:
	compiledb -n make

$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(BIN_DIR)/$@ $(LDFLAGS)
# ar rcs $(BIN_DIR)/$@ $(OBJS)

# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# c sourceB
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@


.PHONY: clean

run: $(TARGET_EXEC)
	cd $(BIN_DIR) && ./$(TARGET_EXEC)


clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

APP      := server
CC       ?= gcc
BUILD    ?= debug

# Standard & Includes
STD      := -std=c23 -D_POSIX_C_SOURCE=200809L
INC      := -Iinclude -Isrc -Ilib/log/src -Ilib/unity/src

# Paranoid Warnings
WARN     := -Wall -Wextra -Werror -Wpedantic -Wshadow -Wconversion -Wstrict-prototypes

# Build Modes
ifeq ($(BUILD),debug)
    CFLAGS  := $(STD) $(WARN) $(INC) -g3 -O0 -DDEBUG -fsanitize=address,undefined
    LDFLAGS := -fsanitize=address,undefined
    OUT     := build/debug
else
    CFLAGS  := $(STD) $(WARN) $(INC) -O3 -DNDEBUG -D_FORTIFY_SOURCE=2 -fstack-protector-strong -fPIE
    LDFLAGS := -pie -Wl,-z,relro,-z,now -s
    OUT     := build/release
endif

# Sources
LOG_SRC  := lib/log/src/log.c
LOG_OBJ  := $(LOG_SRC:%.c=$(OUT)/%.o)

UNITY_SRC:= lib/unity/src/unity.c
UNITY_OBJ:= $(UNITY_SRC:%.c=$(OUT)/%.o)

CORE_SRC := $(filter-out src/main.c, $(wildcard src/*.c))
CORE_OBJ := $(CORE_SRC:%.c=$(OUT)/%.o)

MAIN_SRC := src/main.c
MAIN_OBJ := $(MAIN_SRC:%.c=$(OUT)/%.o)

TEST_SRC := $(wildcard tests/*/*.c)
TEST_BIN := $(TEST_SRC:%.c=$(OUT)/%.bin)

DEPS     := $(LOG_OBJ:.o=.d) $(UNITY_OBJ:.o=.d) $(CORE_OBJ:.o=.d) $(MAIN_OBJ:.o=.d) $(TEST_BIN:.bin=.d)

.PHONY: all clean test

all: $(OUT)/$(APP)

# Main App Link
$(OUT)/$(APP): $(LOG_OBJ) $(CORE_OBJ) $(MAIN_OBJ)
	@mkdir -p $(@D)
	@echo "  [LD] $@"
	@$(CC) $^ -o $@ $(LDFLAGS)

# Test Binary Link (FIXED: Added CFLAGS)
$(OUT)/%.bin: %.c $(CORE_OBJ) $(LOG_OBJ) $(UNITY_OBJ)
	@mkdir -p $(@D)
	@echo "  [LD] $@"
	@$(CC) $(CFLAGS) $< $(CORE_OBJ) $(LOG_OBJ) $(UNITY_OBJ) -o $@ $(LDFLAGS)

# Compile Rule
$(OUT)/%.o: %.c
	@mkdir -p $(@D)
	@echo "  [CC] $<"
	@$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

test: $(TEST_BIN)
	@for t in $(TEST_BIN); do echo "Running $$t"; ./$$t || exit 1; done

clean:
	@rm -rf build

-include $(DEPS)

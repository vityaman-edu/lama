CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O3
LIBS = -lffi -ldl
LDFLAGS = -rdynamic
BUILD_DIR = .vm-build
TARGET = lama.exe
SOURCES = lama.c converter.c vm.c bytecode.c ffi.c loader.c symbols.c ops.c memory.c opcodes.c disasm.c
OBJECTS = $(addprefix $(BUILD_DIR)/,$(SOURCES:.c=.o))

RUNTIME_DIR = ../runtime
RUNTIME_LIB = $(BUILD_DIR)/runtime/runtime.a

ifeq ($(shell uname -s),Darwin)
CC = clang
MACOS_SDK ?= $(shell xcrun --show-sdk-path)
CFLAGS += -arch x86_64 -isysroot $(MACOS_SDK)
CPPFLAGS += -I$(MACOS_SDK)/usr/include/ffi
LDFLAGS = -Wl,-export_dynamic
RUNTIME_LINK = -Wl,-force_load,$(RUNTIME_LIB)
else
RUNTIME_LINK = -Wl,--whole-archive $(RUNTIME_LIB) -Wl,--no-whole-archive
endif

all: $(TARGET)

debug: CFLAGS += -DDEBUG_PRINT -g3 -Og -O0 -fsanitize=address
debug: $(TARGET)

$(TARGET): $(OBJECTS) $(RUNTIME_LIB) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS) $(RUNTIME_LINK) $(LIBS)

$(RUNTIME_LIB):
	$(MAKE) -C $(RUNTIME_DIR) LAMA_ENV=0 BUILD_DIR=$(abspath $(BUILD_DIR)/runtime)

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	$(MAKE) -C $(RUNTIME_DIR) BUILD_DIR=$(abspath $(BUILD_DIR)/runtime) clean
	$(RM) -r $(BUILD_DIR) $(TARGET)

.PHONY: all debug clean

# The directory that contains FreeRTOS source code
FREERTOS_ROOT := ../FreeRTOS/FreeRTOS

# Demo code (current project directory)
DEMO_PROJECT := .

# FreeRTOS kernel
KERNEL_DIR := $(FREERTOS_ROOT)/Source
KERNEL_PORT_DIR := $(KERNEL_DIR)/portable/GCC/ARM_CM3

# Where to store all the generated files (objects, elf and map)
OUTPUT_DIR := ./Output

DEMO_NAME := demo
ELF := $(OUTPUT_DIR)/$(DEMO_NAME).elf
MAP := $(OUTPUT_DIR)/$(DEMO_NAME).map

# Compiler toolchain
CC := arm-none-eabi-gcc
LD := arm-none-eabi-gcc
SIZE := arm-none-eabi-size

# Target embedded board and CPU
MACHINE := mps2-an385 
CPU := cortex-m3

# Use -s to connect to gdb port 1234 and -S to wait before executing
QEMU_FLAGS_DBG = -s -S 

# Include directories
INCLUDE_DIRS = -I$(KERNEL_DIR)/include -I$(KERNEL_PORT_DIR) -I$(DEMO_PROJECT)
VPATH += $(KERNEL_DIR) $(KERNEL_PORT_DIR) $(KERNEL_DIR)/portable/MemMang
VPATH += $(DEMO_PROJECT)

TEST ?= 5

# Compiler flags
CFLAGS = $(INCLUDE_DIRS)
CFLAGS += -DTEST_CASE=$(TEST)
CFLAGS += -ffreestanding
CFLAGS += -mcpu=$(CPU) -mthumb
CFLAGS += -Wall -Wextra -Wshadow
CFLAGS += -g3
CFLAGS += -Os
CFLAGS += -ffunction-sections -fdata-sections

# Linker flags
LDFLAGS = -T ./mps2_m3.ld
LDFLAGS += -nostartfiles -specs=nano.specs -specs=nosys.specs
LDFLAGS += -Xlinker -Map=$(MAP)
LDFLAGS += -Xlinker --gc-sections
LDFLAGS += -mcpu=$(CPU) -mthumb

# FreeRTOS kernel source files
SOURCE_FILES += $(KERNEL_DIR)/queue.c
SOURCE_FILES += $(KERNEL_DIR)/list.c
SOURCE_FILES += $(KERNEL_DIR)/tasks.c
SOURCE_FILES += $(KERNEL_DIR)/portable/MemMang/heap_4.c
SOURCE_FILES += $(KERNEL_DIR)/portable/GCC/ARM_CM3/port.c

# Demo / Project source files
SOURCE_FILES += $(DEMO_PROJECT)/main.c
SOURCE_FILES += $(DEMO_PROJECT)/setup.c
SOURCE_FILES += $(DEMO_PROJECT)/test.c
SOURCE_FILES += $(DEMO_PROJECT)/uart.c

# Utilities (logging) source files
SOURCE_FILES += $(DEMO_PROJECT)/logging.c

# Start-up code
SOURCE_FILES += ./startup.c

# Object files
OBJS = $(SOURCE_FILES:%.c=%.o)
OBJS_NOPATH = $(notdir $(OBJS))
OBJS_OUTPUT = $(OBJS_NOPATH:%.o=$(OUTPUT_DIR)/%.o)

# Default target
all: $(ELF)

# Link ELF
$(ELF): $(OBJS_OUTPUT) ./mps2_m3.ld Makefile
	@echo "\n\n--- Final linking ---\n"
	$(LD) $(LDFLAGS) $(OBJS_OUTPUT) -o $(ELF)
	$(SIZE) $(ELF)

# Compile source to object
$(OUTPUT_DIR)/%.o : %.c  Makefile $(OUTPUT_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create output directory
$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

# Clean object files only
cleanobj:
	rm -f $(OUTPUT_DIR)/*.o

# Clean everything
clean:
	rm -rf $(ELF) $(MAP) $(OUTPUT_DIR)/*.o $(OUTPUT_DIR)

# Run QEMU
qemu_start:
	qemu-system-arm -machine $(MACHINE) -cpu $(CPU) -kernel $(ELF) -monitor none -nographic -serial stdio

qemu_debug:
	qemu-system-arm -machine $(MACHINE) -cpu $(CPU) -kernel $(ELF) -monitor none -nographic -serial stdio $(QEMU_FLAGS_DBG)

# Start GDB
gdb_start:
	gdb-multiarch $(ELF)

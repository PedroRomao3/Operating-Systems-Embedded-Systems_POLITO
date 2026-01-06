# Toolchain
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
CFLAGS = -mcpu=cortex-m4 -mthumb -O0 -g -Wall -I. \
         -I./FreeRTOS/FreeRTOS-Kernel/include \
         -I./FreeRTOS/FreeRTOS-Kernel/portable/GCC/ARM_CM4F \
         -DDEBUG

# Linker script (à adapter selon QEMU STM32 sim)
LDSCRIPT = stm32_flash.ld
LDFLAGS = -T$(LDSCRIPT)

# Source files
SRC = main.c setup.c test.c \
      FreeRTOS/FreeRTOS-Kernel/tasks.c \
      FreeRTOS/FreeRTOS-Kernel/queue.c \
      FreeRTOS/FreeRTOS-Kernel/list.c \
      FreeRTOS/FreeRTOS-Kernel/semphr.c \
      FreeRTOS/FreeRTOS-Kernel/port.c  # selon le port CM4F

OBJ = $(SRC:.c=.o)

# Output
TARGET = main.elf
BIN = main.bin

# Targets
all: $(TARGET) $(BIN)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(TARGET)
	$(OBJCOPY) -O binary $< $@

clean:
	rm -f $(OBJ) $(TARGET) $(BIN)

# Run QEMU (STM32F4 example)
run: $(BIN)
	qemu-system-arm -M stm32-p103 -kernel $(BIN) -nographic -serial mon:stdio

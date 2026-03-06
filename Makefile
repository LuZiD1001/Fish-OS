# ==============================================================================
#  MyOS - Main Build System
#  Compiles bootloader + kernel into a bootable ISO
# ==============================================================================

# Toolchain
CC      = i686-elf-gcc
AS      = nasm
LD      = i686-elf-ld
GRUB    = grub-mkrescue

# Flags
CFLAGS  = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -fno-stack-protector \
           -fno-builtin -nostdlib -Ikernel -Ilibc
ASFLAGS = -f elf32
LDFLAGS = -T kernel/linker.ld -nostdlib

# Source files
BOOT_SRC   = boot/boot.asm
KERNEL_C   = kernel/kernel.c \
              kernel/mm/pmm.c \
              kernel/mm/vmm.c \
              kernel/mm/kheap.c \
              kernel/proc/process.c \
              kernel/proc/scheduler.c \
              kernel/drivers/vga.c \
              kernel/drivers/keyboard.c \
              kernel/drivers/timer.c \
              kernel/fs/vfs.c \
              kernel/fs/ramfs.c \
              kernel/shell.c \
              libc/string.c \
              libc/stdio.c

KERNEL_ASM = kernel/interrupt.asm \
              kernel/gdt.asm \
              kernel/paging.asm

# Object files
OBJS  = $(BOOT_SRC:.asm=.o) \
        $(KERNEL_C:.c=.o) \
        $(KERNEL_ASM:.asm=.o)

# Output
ISO    = MyOS.iso
KERNEL = myos.bin

.PHONY: all clean run debug iso

all: iso

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile ASM files
%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

# Link kernel
$(KERNEL): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

# Build ISO
iso: $(KERNEL)
	cp $(KERNEL) iso/boot/myos.bin
	cp boot/grub.cfg iso/boot/grub/grub.cfg
	$(GRUB) -o $(ISO) iso

# Run in QEMU
run: iso
	qemu-system-i386 -cdrom $(ISO) -m 256M

# Debug with GDB
debug: iso
	qemu-system-i386 -cdrom $(ISO) -m 256M -s -S &
	gdb -ex "target remote :1234" -ex "symbol-file $(KERNEL)"

clean:
	find . -name "*.o" -delete
	rm -f $(KERNEL) $(ISO)

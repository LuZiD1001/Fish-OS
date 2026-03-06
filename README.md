# FishOS – Dein eigenes Betriebssystem

Mein eigenes OS 

---

## Was ist drin?

| Komponente | Datei | Beschreibung |
|---|---|---|
| Bootloader | `boot/boot.asm` | Multiboot-konform, springt in Kernel |
| GDT | `kernel/gdt.c` | Segmenttabelle, Kernel/User Rings |
| IDT | `kernel/idt.c` | 256 Interrupt-Handler + PIC-Remapping |
| ISR/IRQ | `kernel/interrupt.asm` | ASM-Stubs für alle Exceptions + Hardware-IRQs |
| VGA Treiber | `kernel/drivers/vga.c` | 80x25 Farb-Textmodus, printf, Scrolling |
| Keyboard | `kernel/drivers/keyboard.c` | PS/2 Tastatur, Scancode → ASCII, Ring-Buffer |
| Timer (PIT) | `kernel/drivers/timer.c` | 100Hz Tick, sleep() |
| PMM | `kernel/mm/pmm.c` | Bitmap-Allocator für 4KB Pages |
| Heap | `kernel/mm/kheap.c` | kmalloc/kfree mit Free-List |
| VFS | `kernel/fs/vfs.c` | Abstraktionslayer für Dateisysteme |
| RAMFS | `kernel/fs/ramfs.c` | In-Memory Root-Filesystem |
| Shell | `kernel/shell.c` | Interaktive Kommandozeile |
| Prozesse | `kernel/proc/process.c` | Prozessverwaltung (Grundgerüst) |

---

## Voraussetzungen installieren

### Ubuntu / Debian / WSL

```bash
# 1. Cross-Compiler Toolchain (i686-elf-gcc)
sudo apt update
sudo apt install -y build-essential bison flex libgmp-dev libmpc-dev \
                    libmpfr-dev texinfo nasm qemu-system-x86 grub-pc-bin \
                    xorriso mtools

# 2. i686-elf Cross-Compiler bauen (einmalig ~20min)
mkdir -p $HOME/cross && cd $HOME/cross
export PREFIX="$HOME/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

# Binutils
wget https://ftp.gnu.org/gnu/binutils/binutils-2.41.tar.gz
tar xf binutils-2.41.tar.gz
mkdir build-binutils && cd build-binutils
../binutils-2.41/configure --target=$TARGET --prefix=$PREFIX \
    --with-sysroot --disable-nls --disable-werror
make -j$(nproc)
make install
cd ..

# GCC (nur C, kein C++)
wget https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.gz
tar xf gcc-13.2.0.tar.gz
mkdir build-gcc && cd build-gcc
../gcc-13.2.0/configure --target=$TARGET --prefix=$PREFIX \
    --disable-nls --enable-languages=c --without-headers
make -j$(nproc) all-gcc
make -j$(nproc) all-target-libgcc
make install-gcc
make install-target-libgcc

# PATH dauerhaft setzen
echo 'export PATH="$HOME/cross/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

### macOS (Homebrew)

```bash
brew install i686-elf-gcc i686-elf-binutils nasm
brew install qemu
# grub-mkrescue: 
brew install xorriso
# grub selbst musst du aus Source bauen (oder docker nutzen - siehe unten)
```

### Windows → Nutze WSL2!

```powershell
wsl --install
# Dann Ubuntu öffnen und Ubuntu-Anleitung folgen
```

---

## Bauen & Starten

```bash
# In den MyOS Ordner wechseln
cd MyOS

# ISO bauen
make

# In QEMU starten (Emulator)
make run

# Mit GDB debuggen
make debug
# In einem anderen Terminal: gdb myos.bin
# (gdb) target remote :1234
```

---

## Shell-Befehle

Wenn MyOS bootet, siehst du einen Boot-Screen und dann die Shell:

```
root@MyOS:/# help       → Alle Befehle anzeigen
root@MyOS:/# ls         → Dateien auflisten
root@MyOS:/# cat /etc/motd  → Datei lesen
root@MyOS:/# meminfo    → RAM-Statistiken
root@MyOS:/# uptime     → Laufzeit
root@MyOS:/# color      → Farb-Palette
root@MyOS:/# uname      → System-Info
root@MyOS:/# reboot     → Neustart
```

---

## Projektstruktur

```
MyOS/
├── Makefile               ← Build-System
├── boot/
│   ├── boot.asm           ← Bootloader (Multiboot)
│   └── grub.cfg           ← GRUB Konfiguration
├── kernel/
│   ├── kernel.c           ← Kernel-Einstiegspunkt (start hier!)
│   ├── types.h            ← Basis-Typen (uint32_t etc.)
│   ├── gdt.c/h            ← Global Descriptor Table
│   ├── idt.c/h            ← Interrupt Descriptor Table
│   ├── interrupt.asm      ← ISR/IRQ Assembly-Stubs
│   ├── gdt.asm            ← GDT laden (ASM)
│   ├── paging.asm         ← Paging ein/ausschalten
│   ├── shell.c/h          ← Interaktive Shell
│   ├── linker.ld          ← Linker-Script
│   ├── drivers/
│   │   ├── vga.c/h        ← Textmodus-Display
│   │   ├── keyboard.c/h   ← PS/2 Tastatur
│   │   └── timer.c/h      ← PIT Timer
│   ├── mm/
│   │   ├── pmm.c/h        ← Physical Memory Manager
│   │   ├── vmm.c/h        ← Virtual Memory (Stub)
│   │   └── kheap.c/h      ← Kernel Heap (kmalloc/kfree)
│   ├── fs/
│   │   ├── vfs.c/h        ← Virtual Filesystem
│   │   └── ramfs.c/h      ← RAM-Filesystem
│   └── proc/
│       ├── process.c/h    ← Prozessverwaltung
│       └── scheduler.c/h  ← Scheduler (Stub)
└── libc/
    ├── string.c/h         ← strlen, memcpy, strcmp etc.
    └── stdio.h            ← Platzhalter
```

---

## Nächste Schritte (Roadmap)

1. **Paging** – `kernel/mm/vmm.c` ausbauen: echte Page Tables aufbauen
2. **Multitasking** – `scheduler.c` mit Context-Switch in ASM erweitern
3. **ELF-Loader** – ELF-Binaries laden und starten
4. **ATA Treiber** – Von Festplatte lesen/schreiben
5. **FAT32** – Echtes Dateisystem auf Disk
6. **User-Space** – Ring 3, System Calls via `int 0x80`
7. **GUI** – VESA Framebuffer, Fenster, Maus

---

## Lernressourcen

- **OSDev Wiki**: https://wiki.osdev.org (deine Bibel)
- **JamesM's Tutorial**: https://web.archive.org/web/20160412174753/http://www.jamesmolloy.co.uk/tutorial_html/
- **Writing a Simple OS from Scratch**: https://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf
- **Intel x86 Manual**: https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html

---

## Schnell-Einstieg ohne Cross-Compiler (Docker)

```bash
docker run -it --rm -v $(pwd):/os osdev/cross-compiler bash
cd /os
make
```

*(Image muss noch erstellt werden – OSDev-Community hat diverse Vorlagen)*

---

Viel Spaß! Dieses OS ist deine Leinwand. 🖥️

/* ==============================================================================
   MyOS - PS/2 Keyboard Driver (kernel/drivers/keyboard.c)
   Handles IRQ1, translates scancodes to ASCII, buffers keystrokes.
   ============================================================================== */

#include "keyboard.h"
#include "kernel/idt.h"
#include "kernel/drivers/vga.h"

#define KBD_DATA_PORT  0x60
#define KBD_STATUS_PORT 0x64
#define KBD_BUFFER_SIZE 256

static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

/* US QWERTY scancode → ASCII table (Set 1) */
static const char scancode_ascii[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0, 'a','s','d','f','g','h','j','k','l',';','\'','`',
    0, '\\','z','x','c','v','b','n','m',',','.','/', 0,
    '*', 0, ' ', 0,
    0,0,0,0,0,0,0,0,0,0,  /* F1-F10 */
    0, 0,                  /* NumLock, ScrollLock */
    '7','8','9','-','4','5','6','+','1','2','3','0','.',
    0,0,0,
    0,0,                   /* F11, F12 */
};

static const char scancode_shift[128] = {
    0, 27, '!','@','#','$','%','^','&','*','(',')','_','+', '\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0, 'A','S','D','F','G','H','J','K','L',':','"','~',
    0, '|','Z','X','C','V','B','N','M','<','>','?', 0,
    '*', 0, ' '
};

/* Circular keyboard buffer */
static char    kbd_buf[KBD_BUFFER_SIZE];
static uint32_t kbd_head = 0;
static uint32_t kbd_tail = 0;
static bool     shift_held = false;
static bool     caps_lock  = false;

static void kbd_callback(registers_t* regs) {
    (void)regs;
    uint8_t scancode = inb(KBD_DATA_PORT);

    if (scancode & 0x80) {
        /* Key release */
        scancode &= 0x7F;
        if (scancode == 0x2A || scancode == 0x36) shift_held = false;
        return;
    }

    /* Key press */
    if (scancode == 0x2A || scancode == 0x36) {
        shift_held = true;
        return;
    }
    if (scancode == 0x3A) {
        caps_lock = !caps_lock;
        return;
    }

    if (scancode >= 128) return;

    char c;
    if (shift_held) {
        c = scancode_shift[scancode];
    } else {
        c = scancode_ascii[scancode];
        /* Apply caps lock to letters only */
        if (caps_lock && c >= 'a' && c <= 'z')
            c -= 32;
    }

    if (c && ((kbd_head + 1) % KBD_BUFFER_SIZE) != kbd_tail) {
        kbd_buf[kbd_head] = c;
        kbd_head = (kbd_head + 1) % KBD_BUFFER_SIZE;
    }
}

void keyboard_init(void) {
    idt_register_handler(33, kbd_callback);
    vga_printf("[KBD] PS/2 keyboard initialized\n");
}

/* Read one character (blocking) */
char keyboard_getchar(void) {
    while (kbd_head == kbd_tail)
        __asm__ volatile("hlt");  /* Wait for keystroke */

    char c = kbd_buf[kbd_tail];
    kbd_tail = (kbd_tail + 1) % KBD_BUFFER_SIZE;
    return c;
}

/* Read a full line (with echo and backspace) */
int keyboard_readline(char* buf, int max) {
    int i = 0;
    while (i < max - 1) {
        char c = keyboard_getchar();
        if (c == '\n') {
            vga_putchar('\n');
            break;
        } else if (c == '\b') {
            if (i > 0) {
                i--;
                vga_putchar('\b');
                vga_putchar(' ');
                vga_putchar('\b');
            }
        } else {
            buf[i++] = c;
            vga_putchar(c);
        }
    }
    buf[i] = '\0';
    return i;
}

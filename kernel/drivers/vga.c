/* ==============================================================================
   MyOS - VGA Text Mode Driver (kernel/drivers/vga.c)
   ============================================================================== */

#include "vga.h"
#include "libc/string.h"
#include "libc/stdio.h"

static uint16_t* vga_buf;
static uint8_t   vga_col;
static uint8_t   vga_row;
static uint8_t   vga_color;

/* Pack fg/bg color into one byte */
static inline uint8_t make_color(vga_color_t fg, vga_color_t bg) {
    return fg | (bg << 4);
}

/* Pack character + color into a 16-bit VGA entry */
static inline uint16_t make_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

/* Write hardware cursor position via I/O ports */
static void update_cursor(void) {
    uint16_t pos = vga_row * VGA_WIDTH + vga_col;
    __asm__ volatile(
        "outb %0, %1" :: "a"((uint8_t)0x0F), "Nd"((uint16_t)0x3D4)
    );
    __asm__ volatile(
        "outb %0, %1" :: "a"((uint8_t)(pos & 0xFF)), "Nd"((uint16_t)0x3D5)
    );
    __asm__ volatile(
        "outb %0, %1" :: "a"((uint8_t)0x0E), "Nd"((uint16_t)0x3D4)
    );
    __asm__ volatile(
        "outb %0, %1" :: "a"((uint8_t)((pos >> 8) & 0xFF)), "Nd"((uint16_t)0x3D5)
    );
}

void vga_init(void) {
    vga_buf   = VGA_BUFFER;
    vga_col   = 0;
    vga_row   = 0;
    vga_color = make_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_clear();
}

void vga_clear(void) {
    uint16_t blank = make_entry(' ', vga_color);
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        vga_buf[i] = blank;
    vga_col = 0;
    vga_row = 0;
    update_cursor();
}

void vga_set_color(vga_color_t fg, vga_color_t bg) {
    vga_color = make_color(fg, bg);
}

void vga_scroll(void) {
    /* Move all rows up by one */
    for (int row = 1; row < VGA_HEIGHT; row++) {
        for (int col = 0; col < VGA_WIDTH; col++) {
            vga_buf[(row - 1) * VGA_WIDTH + col] =
                vga_buf[row * VGA_WIDTH + col];
        }
    }
    /* Clear last row */
    uint16_t blank = make_entry(' ', vga_color);
    for (int col = 0; col < VGA_WIDTH; col++)
        vga_buf[(VGA_HEIGHT - 1) * VGA_WIDTH + col] = blank;

    vga_row = VGA_HEIGHT - 1;
}

void vga_putchar(char c) {
    if (c == '\n') {
        vga_col = 0;
        vga_row++;
    } else if (c == '\r') {
        vga_col = 0;
    } else if (c == '\t') {
        vga_col = (vga_col + 8) & ~7;
    } else if (c == '\b') {
        if (vga_col > 0) vga_col--;
    } else {
        vga_buf[vga_row * VGA_WIDTH + vga_col] = make_entry(c, vga_color);
        vga_col++;
    }

    /* Wrap line */
    if (vga_col >= VGA_WIDTH) {
        vga_col = 0;
        vga_row++;
    }

    /* Scroll if needed */
    if (vga_row >= VGA_HEIGHT)
        vga_scroll();

    update_cursor();
}

void vga_puts(const char* str) {
    while (*str)
        vga_putchar(*str++);
}

/* Simple printf - supports %s %d %x %c %% */
void vga_printf(const char* fmt, ...) {
    char buf[32];
    __builtin_va_list args;
    __builtin_va_start(args, fmt);

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 's': {
                    const char* s = __builtin_va_arg(args, const char*);
                    vga_puts(s ? s : "(null)");
                    break;
                }
                case 'd': {
                    int n = __builtin_va_arg(args, int);
                    if (n < 0) { vga_putchar('-'); n = -n; }
                    int i = 0;
                    if (n == 0) { vga_putchar('0'); break; }
                    while (n > 0) { buf[i++] = '0' + (n % 10); n /= 10; }
                    while (i--) vga_putchar(buf[i]);
                    break;
                }
                case 'x': {
                    uint32_t n = __builtin_va_arg(args, uint32_t);
                    vga_puts("0x");
                    int i = 0;
                    if (n == 0) { vga_putchar('0'); break; }
                    while (n > 0) {
                        int d = n & 0xF;
                        buf[i++] = d < 10 ? '0' + d : 'a' + d - 10;
                        n >>= 4;
                    }
                    while (i--) vga_putchar(buf[i]);
                    break;
                }
                case 'c':
                    vga_putchar((char)__builtin_va_arg(args, int));
                    break;
                case '%':
                    vga_putchar('%');
                    break;
                default:
                    vga_putchar('%');
                    vga_putchar(*fmt);
            }
        } else {
            vga_putchar(*fmt);
        }
        fmt++;
    }
    __builtin_va_end(args);
}

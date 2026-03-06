/* ==============================================================================
   MyOS - PIT Timer Driver (kernel/drivers/timer.c)
   Programs the 8253/8254 Programmable Interval Timer.
   Fires IRQ0 at configurable Hz. Used for scheduler ticks and sleep().
   ============================================================================== */

#include "timer.h"
#include "kernel/idt.h"
#include "kernel/drivers/vga.h"

#define PIT_CHANNEL0  0x40
#define PIT_CMD       0x43
#define PIT_BASE_FREQ 1193182   /* PIT oscillator frequency in Hz */

static volatile uint64_t tick_count = 0;
static uint32_t ticks_per_second    = 0;

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" :: "a"(val), "Nd"(port));
}

static void timer_callback(registers_t* regs) {
    (void)regs;
    tick_count++;
}

void timer_init(uint32_t hz) {
    ticks_per_second = hz;
    uint32_t divisor = PIT_BASE_FREQ / hz;

    /* Mode 3: square wave generator, channel 0 */
    outb(PIT_CMD, 0x36);
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);

    idt_register_handler(32, timer_callback);
    vga_printf("[TIMER] PIT initialized at %d Hz\n", hz);
}

void timer_sleep(uint32_t ms) {
    uint64_t target = tick_count + (ticks_per_second * ms) / 1000;
    while (tick_count < target)
        __asm__ volatile("hlt");
}

uint64_t timer_get_ticks(void) {
    return tick_count;
}

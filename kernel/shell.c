/* ==============================================================================
   MyOS - Interactive Shell (kernel/shell.c)
   A small command interpreter with built-in commands.
   ============================================================================== */

#include "shell.h"
#include "kernel/drivers/vga.h"
#include "kernel/drivers/keyboard.h"
#include "kernel/drivers/timer.h"
#include "kernel/fs/vfs.h"
#include "kernel/mm/pmm.h"
#include "kernel/mm/kheap.h"
#include "libc/string.h"

#define MAX_CMD_LEN  256
#define MAX_ARGS     16
#define MAX_PATH_LEN 128

static char cwd[MAX_PATH_LEN] = "/";

/* ── Helper: split string into args ── */
static int parse_args(char* line, char* argv[]) {
    int argc = 0;
    char* p = line;
    while (*p && argc < MAX_ARGS) {
        while (*p == ' ') p++;
        if (!*p) break;
        argv[argc++] = p;
        while (*p && *p != ' ') p++;
        if (*p) *p++ = '\0';
    }
    return argc;
}

/* ══════════════════════════════════════
   BUILT-IN COMMANDS
   ══════════════════════════════════════ */

static void cmd_help(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("\n  MyOS Built-in Commands\n");
    vga_puts("  ─────────────────────────────────────\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("  help          Show this help\n");
    vga_puts("  clear         Clear the screen\n");
    vga_puts("  echo <text>   Print text\n");
    vga_puts("  ls [path]     List directory\n");
    vga_puts("  cat <file>    Print file content\n");
    vga_puts("  pwd           Print working directory\n");
    vga_puts("  cd <dir>      Change directory\n");
    vga_puts("  meminfo       Show memory stats\n");
    vga_puts("  uptime        Show system uptime\n");
    vga_puts("  color         Demo color palette\n");
    vga_puts("  reboot        Reboot the system\n");
    vga_puts("  uname         System information\n");
    vga_putchar('\n');
}

static void cmd_clear(void) {
    vga_clear();
}

static void cmd_echo(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        vga_puts(argv[i]);
        if (i < argc - 1) vga_putchar(' ');
    }
    vga_putchar('\n');
}

static void cmd_pwd(void) {
    vga_puts(cwd);
    vga_putchar('\n');
}

static void cmd_ls(int argc, char* argv[]) {
    const char* path = (argc > 1) ? argv[1] : cwd;
    vfs_node_t* dir = vfs_open(path);
    if (!dir) {
        vga_printf("ls: cannot access '%s': No such file or directory\n", path);
        return;
    }
    if (!(dir->flags & VFS_DIRECTORY)) {
        vga_printf("ls: '%s': Not a directory\n", path);
        return;
    }

    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_printf("  %s:\n", path);
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    uint32_t i = 0;
    vfs_node_t* child;
    while ((child = vfs_readdir(dir, i++))) {
        if (child->flags & VFS_DIRECTORY) {
            vga_set_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
            vga_printf("  [DIR]  %s/\n", child->name);
            vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        } else {
            vga_printf("  [FILE] %-24s %d bytes\n", child->name, child->size);
        }
    }
    if (i == 1) vga_puts("  (empty)\n");
}

static void cmd_cat(int argc, char* argv[]) {
    if (argc < 2) { vga_puts("cat: missing operand\n"); return; }

    /* Build full path */
    char path[MAX_PATH_LEN];
    if (argv[1][0] == '/') {
        strncpy(path, argv[1], MAX_PATH_LEN - 1);
    } else {
        strncpy(path, cwd, MAX_PATH_LEN - 1);
        if (path[strlen(path) - 1] != '/')
            strncat(path, "/", MAX_PATH_LEN - strlen(path) - 1);
        strncat(path, argv[1], MAX_PATH_LEN - strlen(path) - 1);
    }

    vfs_node_t* file = vfs_open(path);
    if (!file || (file->flags & VFS_DIRECTORY)) {
        vga_printf("cat: %s: No such file\n", argv[1]);
        return;
    }

    uint8_t buf[512];
    uint32_t offset = 0;
    while (offset < file->size) {
        uint32_t n = vfs_read(file, offset, sizeof(buf), buf);
        if (!n) break;
        for (uint32_t i = 0; i < n; i++)
            vga_putchar((char)buf[i]);
        offset += n;
    }
}

static void cmd_meminfo(void) {
    uint32_t free_pages  = pmm_get_free_pages();
    uint32_t total_pages = pmm_get_total_pages();
    uint32_t used_pages  = total_pages - free_pages;

    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("\n  Memory Information\n");
    vga_puts("  ─────────────────────────────────\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_printf("  Total:  %d MB (%d pages)\n",
               (total_pages * 4096) / (1024 * 1024), total_pages);
    vga_printf("  Used:   %d KB (%d pages)\n",
               (used_pages * 4096) / 1024, used_pages);
    vga_printf("  Free:   %d MB (%d pages)\n",
               (free_pages * 4096) / (1024 * 1024), free_pages);
    vga_putchar('\n');
}

static void cmd_uptime(void) {
    uint64_t ticks = timer_get_ticks();
    uint32_t seconds = (uint32_t)(ticks / 100);  /* Assuming 100Hz */
    vga_printf("Uptime: %d seconds\n", seconds);
}

static void cmd_color(void) {
    vga_puts("\n  VGA Color Palette:\n  ");
    for (int bg = 0; bg < 16; bg++) {
        vga_set_color(bg == 0 ? VGA_COLOR_WHITE : VGA_COLOR_BLACK, bg);
        vga_printf(" %2d ", bg);
    }
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts("\n\n");
}

static void cmd_uname(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("MyOS v0.1 - i686 - Built with GCC\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

static void cmd_reboot(void) {
    vga_puts("Rebooting...\n");
    /* Triple-fault reboot: load invalid IDT, cause exception */
    uint8_t dummy[6] = {0};
    __asm__ volatile("lidt (%0)" :: "r"(dummy));
    __asm__ volatile("int $0");
}

/* ══════════════════════════════════════
   SHELL MAIN LOOP
   ══════════════════════════════════════ */

static void print_prompt(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("root");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_putchar('@');
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("MyOS");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_putchar(':');
    vga_set_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    vga_puts(cwd);
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("# ");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

void shell_run(void) {
    char cmd[MAX_CMD_LEN];
    char* argv[MAX_ARGS];

    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("\n  Welcome to MyOS! Type 'help' for commands.\n\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    while (1) {
        print_prompt();
        keyboard_readline(cmd, MAX_CMD_LEN);

        if (cmd[0] == '\0') continue;

        int argc = parse_args(cmd, argv);
        if (argc == 0) continue;

        if      (strcmp(argv[0], "help")   == 0) cmd_help();
        else if (strcmp(argv[0], "clear")  == 0) cmd_clear();
        else if (strcmp(argv[0], "echo")   == 0) cmd_echo(argc, argv);
        else if (strcmp(argv[0], "pwd")    == 0) cmd_pwd();
        else if (strcmp(argv[0], "ls")     == 0) cmd_ls(argc, argv);
        else if (strcmp(argv[0], "cat")    == 0) cmd_cat(argc, argv);
        else if (strcmp(argv[0], "meminfo")== 0) cmd_meminfo();
        else if (strcmp(argv[0], "uptime") == 0) cmd_uptime();
        else if (strcmp(argv[0], "color")  == 0) cmd_color();
        else if (strcmp(argv[0], "uname")  == 0) cmd_uname();
        else if (strcmp(argv[0], "reboot") == 0) cmd_reboot();
        else {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_printf("  command not found: %s\n", argv[0]);
            vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        }
    }
}

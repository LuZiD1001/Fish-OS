/* ==============================================================================
   MyOS - Virtual Filesystem (kernel/fs/vfs.c)
   Abstraction layer over different filesystem implementations.
   Everything is a file: files, dirs, devices.
   ============================================================================== */

#include "vfs.h"
#include "kernel/mm/kheap.h"
#include "kernel/drivers/vga.h"
#include "libc/string.h"

#define MAX_MOUNTS  8
#define MAX_FDS     64

/* Mount table entry */
typedef struct {
    char       path[64];
    vfs_node_t* root;
    bool        used;
} mount_t;

static mount_t   mounts[MAX_MOUNTS];
static vfs_node_t* fds[MAX_FDS];   /* Open file descriptor table */
static vfs_node_t* vfs_root = NULL;

void vfs_init(void) {
    for (int i = 0; i < MAX_MOUNTS; i++) mounts[i].used = false;
    for (int i = 0; i < MAX_FDS; i++) fds[i] = NULL;
    vga_printf("[VFS] Virtual filesystem initialized\n");
}

void vfs_mount(const char* path, vfs_node_t* root) {
    for (int i = 0; i < MAX_MOUNTS; i++) {
        if (!mounts[i].used) {
            strncpy(mounts[i].path, path, 63);
            mounts[i].root = root;
            mounts[i].used = true;
            if (strcmp(path, "/") == 0)
                vfs_root = root;
            vga_printf("[VFS] Mounted %s at %s\n", root->name, path);
            return;
        }
    }
    vga_printf("[VFS] ERROR: No mount slots available!\n");
}

/* Resolve a path to a VFS node */
vfs_node_t* vfs_open(const char* path) {
    if (!vfs_root) return NULL;
    if (strcmp(path, "/") == 0) return vfs_root;

    /* Simple path walk (no symlinks, no ..) */
    vfs_node_t* node = vfs_root;
    char buf[256];
    strncpy(buf, path, 255);

    char* tok = buf;
    if (*tok == '/') tok++;  /* Skip leading slash */

    while (*tok) {
        char* sep = tok;
        while (*sep && *sep != '/') sep++;
        bool last = (*sep == '\0');
        *sep = '\0';

        /* Find this component in current directory */
        if (!node->readdir || !(node->flags & VFS_DIRECTORY))
            return NULL;

        bool found = false;
        for (uint32_t i = 0; ; i++) {
            vfs_node_t* child = node->readdir(node, i);
            if (!child) break;
            if (strcmp(child->name, tok) == 0) {
                node = child;
                found = true;
                break;
            }
        }
        if (!found) return NULL;
        if (last) return node;
        tok = sep + 1;
    }
    return node;
}

uint32_t vfs_read(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buf) {
    if (node && node->read)
        return node->read(node, offset, size, buf);
    return 0;
}

uint32_t vfs_write(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buf) {
    if (node && node->write)
        return node->write(node, offset, size, buf);
    return 0;
}

vfs_node_t* vfs_readdir(vfs_node_t* node, uint32_t index) {
    if (node && node->readdir && (node->flags & VFS_DIRECTORY))
        return node->readdir(node, index);
    return NULL;
}

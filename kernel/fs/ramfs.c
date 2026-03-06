/* ==============================================================================
   MyOS - RAM Filesystem (kernel/fs/ramfs.c)
   Simple in-memory filesystem. Files live in kernel heap.
   This is your root filesystem at boot.
   ============================================================================== */

#include "ramfs.h"
#include "vfs.h"
#include "kernel/mm/kheap.h"
#include "libc/string.h"
#include "kernel/drivers/vga.h"

#define RAMFS_MAX_CHILDREN 32

typedef struct ramfs_node {
    vfs_node_t           vfs;           /* Must be first */
    struct ramfs_node*   children[RAMFS_MAX_CHILDREN];
    uint32_t             child_count;
    uint8_t*             data;          /* File content */
    uint32_t             capacity;
} ramfs_node_t;

/* VFS callbacks */
static uint32_t ramfs_read(vfs_node_t* node, uint32_t offset,
                            uint32_t size, uint8_t* buf) {
    ramfs_node_t* rn = (ramfs_node_t*)node;
    if (offset >= node->size) return 0;
    if (offset + size > node->size) size = node->size - offset;
    memcpy(buf, rn->data + offset, size);
    return size;
}

static uint32_t ramfs_write(vfs_node_t* node, uint32_t offset,
                             uint32_t size, uint8_t* buf) {
    ramfs_node_t* rn = (ramfs_node_t*)node;
    if (offset + size > rn->capacity) {
        /* Grow buffer */
        uint8_t* new_data = kmalloc(offset + size);
        if (!new_data) return 0;
        if (rn->data) {
            memcpy(new_data, rn->data, node->size);
            kfree(rn->data);
        }
        rn->data     = new_data;
        rn->capacity = offset + size;
    }
    memcpy(rn->data + offset, buf, size);
    if (offset + size > node->size)
        node->size = offset + size;
    return size;
}

static vfs_node_t* ramfs_readdir(vfs_node_t* node, uint32_t index) {
    ramfs_node_t* rn = (ramfs_node_t*)node;
    if (index >= rn->child_count) return NULL;
    return &rn->children[index]->vfs;
}

static ramfs_node_t* ramfs_create_node(const char* name, uint32_t flags) {
    ramfs_node_t* node = (ramfs_node_t*)kcalloc(1, sizeof(ramfs_node_t));
    strncpy(node->vfs.name, name, 63);
    node->vfs.flags    = flags;
    node->vfs.read     = ramfs_read;
    node->vfs.write    = ramfs_write;
    node->vfs.readdir  = ramfs_readdir;
    return node;
}

/* Create initial root filesystem with some default files */
vfs_node_t* ramfs_create_root(void) {
    ramfs_node_t* root = ramfs_create_node("/", VFS_DIRECTORY);

    /* Create /etc directory */
    ramfs_node_t* etc = ramfs_create_node("etc", VFS_DIRECTORY);
    root->children[root->child_count++] = etc;

    /* Create /etc/motd */
    ramfs_node_t* motd = ramfs_create_node("motd", VFS_FILE);
    const char* motd_content =
        "Welcome to MyOS!\n"
        "Type 'help' for a list of commands.\n";
    ramfs_write(&motd->vfs, 0, strlen(motd_content), (uint8_t*)motd_content);
    etc->children[etc->child_count++] = motd;

    /* Create /bin directory */
    ramfs_node_t* bin = ramfs_create_node("bin", VFS_DIRECTORY);
    root->children[root->child_count++] = bin;

    /* Create a README */
    ramfs_node_t* readme = ramfs_create_node("README.txt", VFS_FILE);
    const char* readme_content =
        "MyOS v0.1 - A hobby operating system\n"
        "Built with love and C.\n"
        "\nDirectory structure:\n"
        "  /etc  - System configuration\n"
        "  /bin  - Programs\n";
    ramfs_write(&readme->vfs, 0, strlen(readme_content), (uint8_t*)readme_content);
    root->children[root->child_count++] = readme;

    vga_printf("[RAMFS] Root filesystem created with %d entries\n",
               root->child_count);
    return &root->vfs;
}

#pragma once
#include "kernel/types.h"

#define VFS_FILE        0x01
#define VFS_DIRECTORY   0x02
#define VFS_CHARDEVICE  0x04

struct vfs_node;

typedef uint32_t (*vfs_read_fn)(struct vfs_node*, uint32_t offset, uint32_t size, uint8_t* buf);
typedef uint32_t (*vfs_write_fn)(struct vfs_node*, uint32_t offset, uint32_t size, uint8_t* buf);
typedef struct vfs_node* (*vfs_readdir_fn)(struct vfs_node*, uint32_t index);

typedef struct vfs_node {
    char         name[64];
    uint32_t     flags;
    uint32_t     size;
    void*        data;          /* Filesystem-specific data */
    vfs_read_fn    read;
    vfs_write_fn   write;
    vfs_readdir_fn readdir;
} vfs_node_t;

void        vfs_init(void);
void        vfs_mount(const char* path, vfs_node_t* root);
vfs_node_t* vfs_open(const char* path);
uint32_t    vfs_read(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buf);
uint32_t    vfs_write(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buf);
vfs_node_t* vfs_readdir(vfs_node_t* node, uint32_t index);

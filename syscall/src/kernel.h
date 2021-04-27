#ifndef KERNEL_H
#define KERNEL_H

#define FS_MAX_BYTES 0xffff

typedef enum syscall_e
{
    // create/kill process
    SYS_PROC,
    SYS_KILL,
    
    // memory
    SYS_ALLOC,
    SYS_FREE,
    
    // files
    SYS_FSCREATE,
    SYS_FSDELETE,
    SYS_READ,
    SYS_WRITE,
    SYS_OPEN,
    SYS_CLOSE
} syscall_e;

typedef enum fstype_e
{
    FS_FILE,
    FS_FOLDER
} fstype_e;

typedef struct filesystem_t filesystem_t;

struct filesystem_t
{
    void* data;
    fstype_e type;
    char* name;
    filesystem_t* tree;
    filesystem_t* parent;
};

static filesystem_t* _fs;

void kernel_init();
void kernel_syscall(syscall_e call);

// syscalls
static void _syscall_proc();
static void _syscall_kill();

static void _syscall_alloc();
static void _syscall_free();

static void _syscall_fscreate();
static void _syscall_fsdelete();
static void _syscall_read();
static void _syscall_write();
static void _syscall_open();
static void _syscall_close();


#endif

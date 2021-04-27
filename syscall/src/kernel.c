#include "kernel.h"

#include <stdlib.h>

void kernel_init()
{
    _fs = malloc(sizeof(filesystem_t));
    _fs->data = NULL;
    _fs->type = FS_FOLDER;
    _fs->name = NULL;
    _fs->tree = NULL;
    _fs->parent = NULL;

}

void kernel_syscall(syscall_e call)
{
    switch (call)
    {
        case(SYS_PROC):     _syscall_proc();     break;
        case(SYS_KILL):     _syscall_kill();     break;

        // memory
        case(SYS_ALLOC):    _syscall_alloc();    break;
        case(SYS_FREE):     _syscall_free();     break;

        // files
        case(SYS_FSCREATE): _syscall_fscreate(); break;
        case(SYS_FSDELETE): _syscall_fsdelete(); break;
        case(SYS_READ):     _syscall_read();     break;
        case(SYS_WRITE):    _syscall_write();    break;
        case(SYS_OPEN):     _syscall_open();     break;
        case(SYS_CLOSE):    _syscall_close();    break;
    }
}

// syscalls

void _syscall_proc()
{

}

void _syscall_kill()
{

}


void _syscall_alloc()
{

}

void _syscall_free()
{

}


void _syscall_fscreate()
{

}

void _syscall_fsdelete()
{

}

void _syscall_read()
{

}

void _syscall_write()
{

}

void _syscall_open()
{

}

void _syscall_close()
{

}

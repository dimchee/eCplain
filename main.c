#define LIBC_IMPLEMENTATION
#include "libc.h"

void _start(void) {
    str x = fmt("Arg is %d! %d '%s' works :)", 13, 14, "testing...");
    sys_write(stdout, x.ptr, x.len);
    sys_exit(0);
}

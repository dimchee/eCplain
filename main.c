// from glibc <bits/types>
typedef signed char    i8;
typedef unsigned char  u8;
typedef signed short   i16;
typedef unsigned short u16;
typedef signed int     i32;
typedef unsigned int   u32;
#if defined __x86_64__
typedef signed long   i64;
typedef unsigned long u64;
typedef const char*   c_str;

#define int "You should use u32 or i32"
#define long "You should use u64 or i64"
#define char "You should use u8"

typedef struct {
    u64 len;
    u8* ptr;
} str;

static inline i64 syscall6(i64 nr, i64 a1, i64 a2, i64 a3, i64 a4, i64 a5, i64 a6) {
    i64          ret;
    register i64 r10 asm("r10") = a4;
    register i64 r8 asm("r8")   = a5;
    register i64 r9 asm("r9")   = a6;
    asm volatile("syscall"
                 : "=a"(ret)
                 : "a"(nr), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8), "r"(r9)
                 : "rcx", "r11", "memory");
    return ret;
}
static inline i64 syscall3(i64 nr, i64 a, i64 b, i64 c) {
    i64 ret;
    asm volatile("syscall" : "=a"(ret) : "a"(nr), "D"(a), "S"(b), "d"(c) : "rcx", "r11", "memory");
    return ret;
}
#else
#error "unsuported system!"
#endif
enum {
    stdin  = 0,
    stdout = 1,
    stderr = 2,
};
enum {
    SYS_write  = 1,
    SYS_mmap   = 9,
    SYS_munmap = 11,
    SYS_exit   = 60,
};

static inline i64 sys_write(i32 fd, const void* buf, u64 cnt) {
    return syscall3(SYS_write, fd, (i64)buf, (i64)cnt);
}
static inline i64 sys_exit(i32 status) {
    return syscall3(SYS_exit, status, 0, 0);
}

#define assert(expr)                                                                               \
    ((void)sizeof((expr) ? 1 : 0), __extension__({                                                 \
         if (expr)                                                                                 \
             ;                                                                                     \
         else {                                                                                    \
             sys_write(stderr, "AssertFailed\n", 13);                                              \
             sys_exit(1);                                                                          \
         }                                                                                         \
     }))

// length of string not including sentinel
u64 strlen(const u8* msg) {
    for (u32 i = 0; i < 1e9; i++)
        if (msg[i] == 0) return i;
    return -1;
}

void* alloc(u64 len) {
    i64   pagesz = 4096;
    i64   alloc  = ((len + 1 + pagesz - 1) / pagesz) * pagesz;
    void* mem    = (void*)syscall6(SYS_mmap, (i64)0, (i64)alloc, (i64)3, (i64)34, (i64)-1, (i64)0);
    if ((i64)mem < 0) syscall3(SYS_exit, 1, 0, 0);
    return mem;
}
void free(void* mem) {
    syscall3(SYS_munmap, (i64)mem, (i64)alloc, 0);
}

u8* strcpy(const u8* msg) {
    u64 len = strlen(msg);
    u8* mem = (u8*)alloc(len + 1);
    for (u64 i = 0; i <= len; i++) mem[i] = msg[i];
    return mem;
}

u64 numlen(u32 x) {
    u64 len = 0;
    for (u32 y = x; y != 0; y /= 10) len++;
    return len;
}
void get_num(u32 x, u8* dest, u64* ind) {
    u64 len = numlen(x);
    *ind += len;
    dest[*ind] = 0; // sentinel
    for (u64 i = 1; i <= len; i++, x /= 10) dest[*ind - i] = '0' + x % 10;
}

#define spread(s) s.ptr, s.len
#define slit(s) s, sizeof(s)
#define s_fmt(...) spread(fmt(__VA_ARGS__))
str fmt(c_str fmt, ...) {
    __builtin_va_list args, args_copy;

    c_str fmt2 = fmt;
    u64   len  = 0;
    for (__builtin_va_start(args, fmt), __builtin_va_copy(args_copy, args); *fmt2 != '\0'; fmt2++)
        if (*fmt2 == '%') switch (*(++fmt2)) {
            case 'd': {
                len += numlen(__builtin_va_arg(args, i32));
                break;
            }
            case 'c': {
                len++;
                break;
            }
            case 's': {
                len += strlen(__builtin_va_arg(args, const u8*));
                break;
            }
            default: {
                sys_write(stdout, slit("\nERROR\n"));
                sys_exit(-1);
            }
            }
        else
            len++;
    __builtin_va_end(args);
    u8* sol  = alloc(len + 1);
    sol[len] = '\0';
    u64 i    = 0;
    for (__builtin_va_start(args_copy, fmt); *fmt != '\0'; fmt++)
        if (*fmt == '%') switch (*(++fmt)) {
            case 'd': {
                get_num(__builtin_va_arg(args_copy, i32), sol, &i);
                break;
            }
            case 'c': {
                sol[i++] = (u8) __builtin_va_arg(args_copy, i32);
                break;
            }
            case 's': {
                const u8* s = __builtin_va_arg(args_copy, const u8*);
                for (; *s != '\0'; s++, i++) sol[i] = *s;
                break;
            }
            default: {
                sys_write(stdout, slit("\nERROR\n"));
                sys_exit(-1);
            }
            }
        else
            sol[i++] = *fmt;
    __builtin_va_end(args_copy);
    return (str){.len = len, .ptr = sol};
}

void _start(void) {
    str x = fmt("Arg is %d! %d '%s' works :)", 13, 14, "testing...");
    sys_write(stdout, x.ptr, x.len);
    sys_exit(0);
}

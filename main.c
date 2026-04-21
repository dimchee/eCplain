// from glibc <bits/types>
typedef signed char i8;
typedef unsigned char u8;
typedef signed short i16;
typedef unsigned short u16;
typedef signed int i32;
typedef unsigned int u32;
#if defined __x86_64__
typedef signed long i64;
typedef unsigned long u64;
#else
#error "unsuported system!"
#endif

static inline i64 syscall4(i64 nr, i64 a, i64 b, i64 c) {
  i64 ret;
  asm volatile("syscall"
               : "=a"(ret)
               : "a"(nr), "D"(a), "S"(b), "d"(c)
               : "rcx", "r11", "memory");
  return ret;
}

static inline i64 sys_write(i32 fd, const void *buf, u64 cnt) {
  return syscall4(1, fd, (i64)buf, (i64)cnt);
}

// length of c_str without sentinel
u64 strlen(const char *msg) {
  for (int i = 0; i < 1e9; i++)
    if (msg[i] == 0)
      return i;
  return -1;
}

static inline i64 sys_exit(int status) { return syscall4(60, status, 0, 0); }
i64 print(const char *msg) { return sys_write(1, msg, strlen(msg)); }

void _start(void) {
  print("Hello, World!");
  sys_exit(0);
}

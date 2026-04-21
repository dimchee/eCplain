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

static inline i64 syscall6(i64 nr, i64 a1, i64 a2, i64 a3, i64 a4, i64 a5,
                           i64 a6) {
  i64 ret;
  register i64 r10 asm("r10") = a4;
  register i64 r8 asm("r8") = a5;
  register i64 r9 asm("r9") = a6;
  asm volatile("syscall"
               : "=a"(ret)
               : "a"(nr), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8), "r"(r9)
               : "rcx", "r11", "memory");
  return ret;
}
static inline i64 syscall3(i64 nr, i64 a, i64 b, i64 c) {
  i64 ret;
  asm volatile("syscall"
               : "=a"(ret)
               : "a"(nr), "D"(a), "S"(b), "d"(c)
               : "rcx", "r11", "memory");
  return ret;
}
#else
#error "unsuported system!"
#endif
enum {
  SYS_write = 1,
  SYS_mmap = 9,
  SYS_munmap = 11,
  SYS_exit = 60,
};

static inline i64 sys_write(i32 fd, const void *buf, u64 cnt) {
  return syscall3(SYS_write, fd, (i64)buf, (i64)cnt);
}
static inline i64 sys_exit(int status) {
  return syscall3(SYS_exit, status, 0, 0);
}

// length of c_str without sentinel
u64 strlen(const char *msg) {
  for (int i = 0; i < 1e9; i++)
    if (msg[i] == 0)
      return i;
  return -1;
}

i64 print(const char *msg) { return sys_write(1, msg, strlen(msg)); }

void *alloc(u64 len) {
  i64 pagesz = 4096;
  i64 alloc = ((len + 1 + pagesz - 1) / pagesz) * pagesz;
  void *mem = (void *)syscall6(SYS_mmap, (i64)0, (i64)alloc, (i64)3, (i64)34,
                               (i64)-1, (i64)0);
  if ((i64)mem < 0)
    syscall3(SYS_exit, 1, 0, 0);
  return mem;
}
void free(void *mem) { syscall3(SYS_munmap, (i64)mem, (i64)alloc, 0); }

char *strcpy(const char *msg) {
  u64 len = strlen(msg);
  char *mem = (char *)alloc(len + 1);
  for (u64 i = 0; i <= len; i++)
    mem[i] = msg[i];
  return mem;
}

char *get_num(u32 x) {
  u64 len = 0;
  for (u32 y = x; y != 0; y /= 10)
    len++;
  char *str = alloc(len + 1);
  str[len] = 0; // sentinel
  for (u64 i = 1; i <= len; i++, x /= 10)
    str[len - i] = '0' + x % 10;
  return str;
}

// ToDo varargs
void printf(const char *fmt, u32 x) {
  char *num = get_num(x);
  u64 num_len = strlen(num);
  char *s = alloc(strlen(fmt) + num_len + 1 - 2);

  for (i32 i = 0, j = 0; j < strlen(fmt); i++, j++) {
    s[i] = fmt[j];
    if (fmt[j + 1] == '%' && fmt[j + 2] == 'd') {
      for (i32 k = 0; k < num_len; k++)
        s[++i] = num[k];
      j += 2;
    }
  }
  print(s);
  free(s);
}

void _start(void) {
  printf("Arg is %d!", 13);
  sys_exit(0);
}

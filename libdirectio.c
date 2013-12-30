#define _GNU_SOURCE
#define __USE_GNU

#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdlib.h>

#ifdef DEBUG
#define DPRINTF(format, args...) fprintf(stderr, "debug: " format, ##args)
#else
#define DPRINTF(format, args...)
#endif

int open(const char *, int, ...) __attribute__ ((weak, alias("wrap_open")));
int __open(const char *, int, ...) __attribute__ ((weak, alias("wrap_open")));
int open64(const char *, int, ...) __attribute__ ((weak, alias("wrap_open64")));
int __open64(const char *, int, ...) __attribute__ ((weak, alias("wrap_open64")));

static int (*orig_open)(const char *, int, ...) = NULL;
static int (*orig_open64)(const char *, int, ...) = NULL;

static int __do_wrap_open(const char *name, int flags, mode_t mode,
int (*func_open)(const char *, int, ...))
{
    int ret;

    // certain filesystems don't support O_DIRECT, return EINVAL
    ret = func_open(name, flags | O_DIRECT, mode);
    if(ret == -1 && errno == EINVAL) {
        ret = func_open(name, flags, mode);
    }
    return ret;
}

int wrap_open(const char *name, int flags, ...)
{
    va_list args;
    mode_t mode;

    va_start(args, flags);
    mode = va_arg(args, mode_t);
    va_end(args);

    DPRINTF("calling libc open(%s, 0x%x, 0x%x)\n", name, flags, mode);

    return __do_wrap_open(name, flags, mode, orig_open);
}

int wrap_open64(const char *name, int flags, ...)
{
    va_list args;
    mode_t mode;

    va_start(args, flags);
    mode = va_arg(args, mode_t);
    va_end(args);

    DPRINTF("calling libc open64(%s, 0x%x, 0x%x)\n", name, flags, mode);

    return __do_wrap_open(name, flags, mode, orig_open64);
}

void _init(void)
{
    orig_open = dlsym(RTLD_NEXT, "open");
    if (!orig_open) {
        fprintf(stderr, "error: missing symbol open!\n");
        exit(1);
    }
    orig_open64 = dlsym(RTLD_NEXT, "open64");
    if (!orig_open64) {
        fprintf(stderr, "error: missing symbol open64!\n");
        exit(1);
    }
}

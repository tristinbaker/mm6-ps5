// Makes a Linux process see files the way a PS5 native title does, to catch startup failures on the desktop:
// the working directory can't be queried, and relative paths fail with EINVAL. Absolute paths work as usual.
//
//     cc -shared -fPIC -O2 -o ps5-fs-sim.so ps5-fs-sim.c -ldl
//     LD_PRELOAD=$PWD/ps5-fs-sim.so OpenEnroth --headless
#define _GNU_SOURCE
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int relative(const char *path) {
    return path && path[0] != '/';
}

static void report(const char *function, const char *path) {
    static int reported;
    if (reported < 50) {
        reported++;
        fprintf(stderr, "[ps5-fs-sim] %s(\"%s\") -> EINVAL\n", function, path ? path : "(null)");
    }
}

#define REAL(name) static __typeof__(name) *real_##name; if (!real_##name) real_##name = dlsym(RTLD_NEXT, #name)

char *getcwd(char *buffer, size_t size) {
    (void) buffer; (void) size;
    report("getcwd", "");
    errno = EINVAL;
    return NULL;
}

char *get_current_dir_name(void) {
    report("get_current_dir_name", "");
    errno = EINVAL;
    return NULL;
}

#define STAT_LIKE(name, type)                                                       \
    int name(const char *path, type *st) {                                          \
        REAL(name);                                                                 \
        if (relative(path)) { report(#name, path); errno = EINVAL; return -1; }     \
        return real_##name(path, st);                                               \
    }
STAT_LIKE(stat, struct stat)
STAT_LIKE(lstat, struct stat)
STAT_LIKE(stat64, struct stat64)
STAT_LIKE(lstat64, struct stat64)

int fstatat(int dirfd, const char *path, struct stat *st, int flags) {
    REAL(fstatat);
    if (relative(path) && !(flags & AT_EMPTY_PATH && path[0] == 0)) { report("fstatat", path); errno = EINVAL; return -1; }
    return real_fstatat(dirfd, path, st, flags);
}

int fstatat64(int dirfd, const char *path, struct stat64 *st, int flags) {
    REAL(fstatat64);
    if (relative(path) && !(flags & AT_EMPTY_PATH && path[0] == 0)) { report("fstatat64", path); errno = EINVAL; return -1; }
    return real_fstatat64(dirfd, path, st, flags);
}

int statx(int dirfd, const char *path, int flags, unsigned mask, struct statx *st) {
    REAL(statx);
    if (relative(path) && !(flags & AT_EMPTY_PATH && path[0] == 0)) { report("statx", path); errno = EINVAL; return -1; }
    return real_statx(dirfd, path, flags, mask, st);
}

int access(const char *path, int mode) {
    REAL(access);
    if (relative(path)) { report("access", path); errno = EINVAL; return -1; }
    return real_access(path, mode);
}

#define OPEN_LIKE(name)                                                             \
    int name(const char *path, int flags, ...) {                                    \
        REAL(name);                                                                 \
        va_list args; va_start(args, flags); int mode = va_arg(args, int); va_end(args); \
        if (relative(path)) { report(#name, path); errno = EINVAL; return -1; }     \
        return real_##name(path, flags, mode);                                      \
    }
OPEN_LIKE(open)
OPEN_LIKE(open64)

#define OPENAT_LIKE(name)                                                           \
    int name(int dirfd, const char *path, int flags, ...) {                         \
        REAL(name);                                                                 \
        va_list args; va_start(args, flags); int mode = va_arg(args, int); va_end(args); \
        if (relative(path)) { report(#name, path); errno = EINVAL; return -1; }     \
        return real_##name(dirfd, path, flags, mode);                               \
    }
OPENAT_LIKE(openat)
OPENAT_LIKE(openat64)

FILE *fopen(const char *path, const char *mode) {
    REAL(fopen);
    if (relative(path)) { report("fopen", path); errno = EINVAL; return NULL; }
    return real_fopen(path, mode);
}

FILE *fopen64(const char *path, const char *mode) {
    REAL(fopen64);
    if (relative(path)) { report("fopen64", path); errno = EINVAL; return NULL; }
    return real_fopen64(path, mode);
}

DIR *opendir(const char *path) {
    REAL(opendir);
    if (relative(path)) { report("opendir", path); errno = EINVAL; return NULL; }
    return real_opendir(path);
}

#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "helper.h"

int open(const char* path, int oflag, ...) {
    static int (*oldfunc)(const char*, int, ...) = NULL;
    static FILE* fp = NULL;

    if (oldfunc == NULL) oldfunc = getOldFunc("open");
    if (fp == NULL) fp = fdopen(3, "w");

    int ret = -1;
    int mode = 0;

    if (__OPEN_NEEDS_MODE(oflag)) {
        va_list args;
        va_start(args, oflag);
        mode = va_arg(args, int);
        va_end(args);
    }

    if (oldfunc != NULL) {
        ret = oldfunc(path, oflag, mode);
        fprintf(fp, "[logger] open(\"%s\", %03o, %03o) = %d\n", getAbsPath(path), oflag, mode, ret);
    }
    return ret;
}

int close(int fd) {
    static int (*oldfunc)(int) = NULL;
    static FILE* fp = NULL;

    if (oldfunc == NULL) oldfunc = getOldFunc("close");
    if (fp == NULL) fp = fdopen(3, "w");

    int ret = -1;

    if (oldfunc != NULL) {
        const char* filename = getFilename(fd);
        ret = oldfunc(fd);
        fprintf(fp, "[logger] close(\"%s\") = %d\n", filename, ret);
    }
    return ret;
}

int creat(const char* pathname, mode_t mode) {
    static int (*oldfunc)(const char*, mode_t) = NULL;
    static FILE* fp = NULL;

    if (oldfunc == NULL) oldfunc = getOldFunc("creat");
    if (fp == NULL) fp = fdopen(3, "w");

    int ret = -1;

    if (oldfunc != NULL) {
        ret = oldfunc(pathname, mode);
        fprintf(fp, "[logger] creat(\"%s\", %03o) = %d\n", getAbsPath(pathname), mode, ret);
    }
    return ret;
}

ssize_t read(int fildes, void* buf, size_t nbyte) {
    static ssize_t(*oldfunc)(int, void*, size_t) = NULL;
    static FILE* fp = NULL;

    if (oldfunc == NULL) oldfunc = getOldFunc("read");
    if (fp == NULL) fp = fdopen(3, "w");

    ssize_t ret = -1;

    if (oldfunc != NULL) {
        ret = oldfunc(fildes, buf, nbyte);
        fprintf(fp, "[logger] read(\"%s\", \"%s\", %ld) = %ld\n", getFilename(fildes), getOutputString(buf, nbyte), nbyte, ret);
    }
    return ret;
}

ssize_t write(int fildes, const void* buf, size_t nbyte) {
    static ssize_t(*oldfunc)(int, const void*, size_t) = NULL;
    static FILE* fp = NULL;

    if (oldfunc == NULL) oldfunc = getOldFunc("write");
    if (fp == NULL) fp = fdopen(3, "w");

    ssize_t ret = -1;

    if (oldfunc != NULL) {
        ret = oldfunc(fildes, buf, nbyte);
        fprintf(fp, "[logger] write(\"%s\", \"%s\", %ld) = %ld\n", getFilename(fildes), getOutputString(buf, nbyte), nbyte, ret);
    }
    return ret;
}

int remove(const char* pathname) {
    static int (*oldfunc)(const char*) = NULL;
    static FILE* fp = NULL;

    if (oldfunc == NULL) oldfunc = getOldFunc("remove");
    if (fp == NULL) fp = fdopen(3, "w");

    int ret = -1;

    if (oldfunc != NULL) {
        const char* filename = getAbsPath(pathname);
        ret = oldfunc(pathname);
        fprintf(fp, "[logger] remove(\"%s\") = %d\n", filename, ret);
    }
    return ret;
}

int rename(const char* old, const char* new) {
    static int (*oldfunc)(const char*, const char*) = NULL;
    static FILE* fp = NULL;

    if (oldfunc == NULL) oldfunc = getOldFunc("rename");
    if (fp == NULL) fp = fdopen(3, "w");

    int ret = -1;

    if (oldfunc != NULL) {
        char oldname[MAXLINE] = {};
        strncpy(oldname, getAbsPath(old), MAXLINE);
        ret = oldfunc(old, new);
        fprintf(fp, "[logger] remove(\"%s\", \"%s\") = %d\n", oldname, getAbsPath(new), ret);
    }
    return ret;
}

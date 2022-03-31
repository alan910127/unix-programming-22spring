#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAXLINE 4096
#define err_quit(msg) { perror(msg); exit(EXIT_FAILURE); }
typedef const char* const string;

#pragma region functions_pointers

static int (*old_chmod)(const char*, mode_t) = NULL;
static int (*old_chown)(const char*, uid_t, gid_t) = NULL;
static int (*old_close)(int) = NULL;
static int (*old_creat)(const char*, mode_t) = NULL;
static int (*old_fclose)(FILE*) = NULL;
static FILE* (*old_fopen)(const char*, const char*) = NULL;
static size_t(*old_fread)(void*, size_t, size_t, FILE*) = NULL;
static size_t(*old_fwrite)(const void*, size_t, size_t, FILE*) = NULL;
static int (*old_open)(const char*, int, ...) = NULL;
static ssize_t(*old_read)(int, void*, size_t) = NULL;
static int (*old_remove)(const char*) = NULL;
static int (*old_rename)(const char*, const char*) = NULL;
static FILE* (*old_tmpfile)(void) = NULL;
static ssize_t(*old_write)(int, const void*, size_t) = NULL;

#pragma endregion

#pragma region helper

char buf[MAXLINE] = {};

void* get_oldfunc(string funcname) {
    void* handle = dlopen("libc.so.6", RTLD_LAZY);
    return (handle == NULL) ? NULL : dlsym(handle, funcname);
}

string get_abspath(string relativepath) {
    bzero(buf, MAXLINE);
    string ptr = realpath(relativepath, buf);
    return (ptr == NULL) ? relativepath : ptr;
}

char* get_filename(int fd) {
    bzero(buf, MAXLINE);
    char fdpath[MAXLINE] = {};
    snprintf(fdpath, MAXLINE, "/proc/%d/fd/%d", getpid(), fd);
    if (readlink(fdpath, buf, MAXLINE) < 0) err_quit("readlink");
    return buf;
}

#define MAXOUTPUT 32

string get_outputstring(string ptr) {
    static char str[MAXOUTPUT + 1];
    bzero(str, MAXOUTPUT + 1);
    strncpy(str, ptr, MAXOUTPUT);
    for (int i = 0; i < MAXOUTPUT && str[i] != '\0'; ++i) {
        if (!isprint(str[i])) str[i] = '.';
    }
    return str;
}

#pragma endregion

#pragma region permission

int chmod(const char* path, mode_t mode) {

    if (old_chmod == NULL) old_chmod = get_oldfunc("chmod");

    int ret = -1;

    if (old_chmod != NULL) {
        ret = old_chmod(path, mode);
        fprintf(stderr, "[logger] chmod(\"%s\", %03o) = %d\n", get_abspath(path), mode, ret);
    }

    return ret;
}

int chown(const char* path, uid_t owner, gid_t group) {
    if (old_chown == NULL) old_chown = get_oldfunc("chown");

    int ret = -1;

    if (old_chown != NULL) {
        ret = old_chown(path, owner, group);
        fprintf(stderr, "[logger] chown(\"%s\", %u, %u) = %d\n", get_abspath(path), owner, group, ret);
    }

    return ret;
}

#pragma endregion

#pragma region file_syscall

int open(const char* path, int oflag, ...) {
    if (old_open == NULL) old_open = get_oldfunc("open");

    int ret = -1;

    if (old_open != NULL) {
        va_list args;
        va_start(args, oflag);
        int optarg = 0;
        optarg = va_arg(args, int);

        ret = old_open(path, oflag, optarg);
        fprintf(stderr, "[logger] open(\"%s\", %03o, %03o) = %d\n", get_abspath(path), oflag, optarg, ret);

        va_end(args);
    }

    return ret;
}

int close(int fd) {
    if (old_close == NULL) old_close = get_oldfunc("close");

    int ret = -1;

    if (old_close != NULL) {
        string str = get_filename(fd);
        ret = old_close(fd);
        fprintf(stderr, "[logger] close(\"%s\") = %d\n", str, ret);
    }

    return ret;
}

int creat(const char* pathname, mode_t mode) {
    if (old_creat == NULL) old_creat = get_oldfunc("creat");

    int ret = -1;

    if (old_creat != NULL) {
        ret = old_creat(pathname, mode);
        fprintf(stderr, "[logger] creat(\"%s\", %03o) = %d\n", get_abspath(pathname), mode, ret);
    }

    return ret;
}

ssize_t read(int fildes, void* buf, size_t nbyte) {
    if (old_read == NULL) old_read = get_oldfunc("read");

    ssize_t ret = -1;

    if (old_read != NULL) {
        ret = old_read(fildes, buf, nbyte);
        fprintf(stderr, "[logger] read(\"%s\", \"%s\", %ld) = %ld\n", get_filename(fildes), get_outputstring(buf), nbyte, ret);
    }

    return ret;
}

ssize_t write(int fildes, const void* buf, size_t nbyte) {
    if (old_write == NULL) old_write = get_oldfunc("write");

    ssize_t ret = -1;

    if (old_write != NULL) {
        ret = old_write(fildes, buf, nbyte);
        fprintf(stderr, "[logger] write(\"%s\", \"%s\", %ld) = %ld\n", get_filename(fildes), get_outputstring(buf), nbyte, ret);
    }

    return ret;
}

int remove(const char* pathname) {
    if (old_remove == NULL) old_remove = get_oldfunc("remove");

    int ret = -1;

    if (old_remove != NULL) {
        string str = get_abspath(pathname);
        ret = old_remove(pathname);
        fprintf(stderr, "[logger] remove(\"%s\") = %d\n", str, ret);
    }

    return ret;
}

int rename(const char* old, const char* new) {
    if (old_rename == NULL) old_rename = get_oldfunc("rename");

    int ret = -1;

    if (old_rename != NULL) {
        char old_path[MAXLINE] = {};
        strncpy(old_path, get_abspath(old), MAXLINE);
        ret = old_rename(old, new);
        fprintf(stderr, "[logger] rename(\"%s\", \"%s\") = %d\n", old_path, get_abspath(new), ret);
    }

    return ret;
}

#pragma endregion

#pragma region file_stdio

FILE* fopen(const char* pathname, const char* mode) {
    if (old_fopen == NULL) old_fopen = get_oldfunc("fopen");

    FILE* ret = NULL;

    if (old_fopen != NULL) {
        ret = old_fopen(pathname, mode);
        fprintf(stderr, "[logger] fopen(\"%s\", \"%s\") = %p\n", get_abspath(pathname), mode, ret);
    }

    return ret;
}

int fclose(FILE* stream) {
    if (old_fclose == NULL) old_fclose = get_oldfunc("fclose");

    int ret = -1;

    if (old_fclose != NULL) {
        string str = get_filename(fileno(stream));
        ret = old_fclose(stream);
        fprintf(stderr, "[logger] fclose(\"%s\") = %d\n", str, ret);
    }

    return ret;
}

size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    if (old_fread == NULL) old_fread = get_oldfunc("fread");

    size_t ret = 0;

    if (old_fread != NULL) {
        ret = old_fread(ptr, size, nmemb, stream);
        fprintf(stderr, "[logger] fread(\"%s\", %ld, %ld, \"%s\") = %ld\n", get_outputstring(ptr), size, nmemb, get_filename(fileno(stream)), ret);
    }

    return ret;
}

size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream) {
    if (old_fwrite == NULL) old_fwrite = get_oldfunc("fwrite");

    size_t ret = 0;

    if (old_fwrite != NULL) {
        ret = old_fwrite(ptr, size, nmemb, stream);
        fprintf(stderr, "[logger] fwrite(\"%s\", %ld, %ld, \"%s\") = %ld\n", get_outputstring(ptr), size, nmemb, get_filename(fileno(stream)), ret);
    }

    return ret;
}

FILE* tmpfile(void) {
    if (old_tmpfile == NULL) old_tmpfile = get_oldfunc("tmpfile");

    FILE* ret = NULL;

    if (old_tmpfile != NULL) {
        ret = old_tmpfile();
        fprintf(stderr, "[logger] tmpfile() = %p\n", ret);
    }

    return ret;
}

#pragma endregion

#include "helper.h"

void* getOldFunc(const char* function) {
    void* handle = dlopen("libc.so.6", RTLD_LAZY);
    return handle ? dlsym(handle, function) : NULL;
}

const char* getAbsPath(const char* relativepath) {
    static char buf[MAXLINE];
    bzero(buf, MAXLINE);
    const char* ptr = realpath(relativepath, buf);
    return ptr ? ptr : relativepath;
}

const char* getFilename(int fd) {
    static char buf[MAXLINE];
    bzero(buf, MAXLINE);
    char tmp[MAXLINE] = {};
    snprintf(tmp, MAXLINE, "/proc/self/fd/%d", fd);
    if (readlink(tmp, buf, MAXLINE) < 0) {
        perror("readlink");
        _exit(-1);
    }
    return buf;
}

const char* getOutputString(const char* ptr, size_t size) {
    static char str[MAX_OUTPUT + 1];
    bzero(str, MAX_OUTPUT + 1);
    for (int i = 0; i < MIN(size, MAX_OUTPUT) && ptr[i] != '\0'; ++i) {
        str[i] = isprint(ptr[i]) ? ptr[i] : '.';
    }
    return str;
}
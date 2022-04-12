#include <unistd.h>

#include "helper.h"

int chmod(const char* path, mode_t mode) {
    static int (*oldfunc)(const char*, mode_t) = NULL;

    if (oldfunc == NULL) oldfunc = getOldFunc("chmod");
    if (fp == NULL) fp = fdopen(3, "w");

    int ret = -1;
    if (oldfunc != NULL) {
        ret = oldfunc(path, mode);
        fprintf(fp, "[logger] chmod(\"%s\", %03o) = %d\n", getAbsPath(path), mode, ret);
    }
    return ret;
}

int chown(const char* path, uid_t owner, gid_t group) {
    static int (*oldfunc)(const char*, uid_t, gid_t) = NULL;

    if (oldfunc == NULL) oldfunc = getOldFunc("chown");
    if (fp == NULL) fp = fdopen(3, "w");

    int ret = -1;
    if (oldfunc != NULL) {
        ret = oldfunc(path, owner, group);
        fprintf(fp, "[logger] chown(\"%s\", %u, %u) = %d\n", getAbsPath(path), owner, group, ret);
    }
    return ret;
}

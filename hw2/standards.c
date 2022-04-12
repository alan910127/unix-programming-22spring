#include "helper.h"

FILE* fopen(const char* pathname, const char* mode) {
    static FILE* (*oldfunc)(const char*, const char*) = NULL;

    if (oldfunc == NULL) oldfunc = getOldFunc("fopen");
    if (fp == NULL) fp = fdopen(3, "w");

    FILE* ret = NULL;

    if (oldfunc != NULL) {
        ret = oldfunc(pathname, mode);
        fprintf(fp, "[logger] fopen(\"%s\", \"%s\") = %p\n", getAbsPath(pathname), mode, ret);
    }
    return ret;
}

int fclose(FILE* stream) {
    static int (*oldfunc)(FILE*) = NULL;

    if (oldfunc == NULL) oldfunc = getOldFunc("fclose");
    if (fp == NULL) fp = fdopen(3, "w");

    int ret = -1;

    if (oldfunc != NULL) {
        const char* filename = getFilename(fileno(stream));
        ret = oldfunc(stream);
        fprintf(fp, "[logger] fclose(\"%s\") = %d\n", filename, ret);
    }
    return ret;
}

size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    static size_t(*oldfunc)(void*, size_t, size_t, FILE*) = NULL;

    if (oldfunc == NULL) oldfunc = getOldFunc("fread");
    if (fp == NULL) fp = fdopen(3, "w");

    size_t ret = 0;

    if (oldfunc != NULL) {
        ret = oldfunc(ptr, size, nmemb, stream);
        fprintf(fp, "[logger] fread(\"%s\", %ld, %ld, \"%s\") = %ld\n", getOutputString(ptr, nmemb), size, nmemb, getFilename(fileno(stream)), ret);
    }
    return ret;
}

size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream) {
    static size_t(*oldfunc)(const void*, size_t, size_t, FILE*) = NULL;

    if (oldfunc == NULL) oldfunc = getOldFunc("fwrite");
    if (fp == NULL) fp = fdopen(3, "w");

    size_t ret = 0;

    if (oldfunc != NULL) {
        ret = oldfunc(ptr, size, nmemb, stream);
        fprintf(fp, "[logger] fwrite(\"%s\", %ld, %ld, \"%s\") = %ld\n", getOutputString(ptr, nmemb), size, nmemb, getFilename(fileno(stream)), ret);
    }
    return ret;
}

FILE* tmpfile(void) {
    static FILE* (*oldfunc)(void) = NULL;

    if (oldfunc == NULL) oldfunc = getOldFunc("tmpfile");
    if (fp == NULL) fp = fdopen(3, "w");

    FILE* ret = NULL;

    if (oldfunc != NULL) {
        ret = oldfunc();
        fprintf(fp, "[logger] tmpfile() = %p\n", ret);
    }
    return ret;
}
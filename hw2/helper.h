#ifndef _HELPER_H
#define _HELPER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <dlfcn.h>
#include <unistd.h>

#define MAXLINE 4096
#define MAX_OUTPUT 32
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

void* getOldFunc(const char* function);
const char* getAbsPath(const char* relativepath);
const char* getFilename(int fd);
const char* getOutputString(const char* ptr, size_t size);

#endif // !_HELPER_H
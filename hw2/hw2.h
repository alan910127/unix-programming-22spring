#ifndef _HW2_H
#define _HW2_H

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAXLINE 4096
#define errquit(msg) { perror(msg); _exit(-1); }

#endif // !_HW2_H
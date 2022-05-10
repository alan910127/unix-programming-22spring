#ifndef _LIBMINI_H
#define _LIBMINI_H

#include "signal.h"

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);

sighandler_t signal(int signum, sighandler_t handler);
int sigaction(int signum, const struct sigaction* act, struct sigaction* oldact);

int sigprocmask(int how, const sigset_t* set, sigset_t* oldset);

int sigpending(sigset_t* set);

unsigned int alarm(unsigned int sec);

ssize_t write(int fd, const void* buf, size_t count);

int pause(void);

unsigned int sleep(unsigned int seconds);

void exit(int status);

size_t strlen(const char* s);

int sigemptyset(sigset_t* set);
int sigfillset(sigset_t* set);
int sigaddset(sigset_t* set, int sig);
int sigdelset(sigset_t* set, int sig);
int sigismember(const sigset_t* set, int sig);

#endif /* _LIBMINI_H */
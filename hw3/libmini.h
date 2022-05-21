#ifndef _LIBMINI_H
#define _LIBMINI_H

#include "signal.h"

extern int setjmp(jmp_buf env);
extern void longjmp(jmp_buf env, int val);

sighandler_t signal(int signum, sighandler_t handler);
int sigaction(int signum, struct sigaction* act, struct sigaction* oldact);
int sigprocmask(int how, const sigset_t* set, sigset_t* oldset);
int sigpending(sigset_t* set);

void __myrt(void);
extern int sys_rt_sigaction(int signum, const struct sigaction* act, struct sigaction* oldact, size_t sigsetsize);
extern int sys_rt_sigprocmask(int how, const sigset_t* set, sigset_t* oldset, size_t sigsetsize);
extern int sys_rt_sigpending(sigset_t* set, size_t sigsetsize);

unsigned int alarm(unsigned int sec);

extern ssize_t write(int fd, const void* buf, size_t count);

extern int sys_pause(void);
int pause(void);

extern unsigned int sleep(unsigned int seconds);

extern void exit(int status);

size_t strlen(const char* s);

int sigemptyset(sigset_t* set);
int sigfillset(sigset_t* set);
int sigaddset(sigset_t* set, int sig);
int sigdelset(sigset_t* set, int sig);
int sigismember(const sigset_t* set, int sig);

void perror(const char* s);

#endif /* _LIBMINI_H */
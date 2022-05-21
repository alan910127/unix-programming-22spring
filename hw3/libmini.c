#include "libmini.h"

size_t strlen(const char* s) {
    size_t len = 0;
    while (s[len] != '\0') ++len;
    return len;
}

int pause(void) {
    long ret = sys_pause();
    return ret;
}

int sigemptyset(sigset_t* set) {
    for (int i = 0; i < _NSIG_WORDS; ++i) {
        set->sig[i] = 0;
    }
    return 0;
}

int sigfillset(sigset_t* set) {
    for (int i = 0; i < _NSIG_WORDS; ++i) {
        set->sig[i] = (~0);
    }
    return 0;
}

int sigaddset(sigset_t* set, int sig) {
    unsigned long _sig = sig - 1;
    set->sig[_sig / _NSIG_BPW] |= 1UL << (_sig % _NSIG_BPW);
    return 0;
}

int sigdelset(sigset_t* set, int sig) {
    unsigned long _sig = sig - 1;
    set->sig[_sig / _NSIG_BPW] &= ~(1UL << (_sig % _NSIG_BPW));
    return 0;
}

int sigismember(const sigset_t* set, int sig) {
    unsigned long _sig = sig - 1;
    return 1 & (set->sig[_sig / _NSIG_BPW] >> (_sig % _NSIG_BPW));
}

sighandler_t signal(int signum, sighandler_t handler) {
    struct sigaction act, oact;

    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if (sigaction(signum, &act, &oact) < 0) return SIG_ERR;
    return oact.sa_handler;
}

int sigaction(int signum, struct sigaction* act, struct sigaction* oldact) {

    if (act) {
        act->sa_flags |= SA_RESTORER;
        act->sa_restorer = __myrt;
    }
    long ret = sys_rt_sigaction(signum, act, oldact, sizeof(sigset_t));
    return ret;
}

int sigprocmask(int how, const sigset_t* set, sigset_t* oldset) {
    long ret = sys_rt_sigprocmask(how, set, oldset, sizeof(sigset_t));
    return ret;
}

int sigpending(sigset_t* set) {
    long ret = sys_rt_sigpending(set, sizeof(sigset_t));
    return ret;
}

void perror(const char* s) {
    write(2, s, strlen(s));
}
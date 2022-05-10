#include "libmini.h"

size_t strlen(const char* s) {
    size_t len = 0;
    while (s[len] != '\0') ++len;
    return len;
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

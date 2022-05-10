#ifndef _SIGNAL_H
#define _SIGNAL_H

#if defined(__x86_64__) && !defined(__ILP32__)
# define __BITS_PER_LONG 64
#else
# define __BITS_PER_LONG 32
#endif

#define _NSIG		64
#define _NSIG_BPW	__BITS_PER_LONG
#define _NSIG_WORDS	(_NSIG / _NSIG_BPW)

#define SIGHUP		 1
#define SIGINT		 2
#define SIGQUIT		 3
#define SIGILL		 4
#define SIGTRAP		 5
#define SIGABRT		 6
#define SIGIOT		 6
#define SIGBUS		 7
#define SIGFPE		 8
#define SIGKILL		 9
#define SIGUSR1		10
#define SIGSEGV		11
#define SIGUSR2		12
#define SIGPIPE		13
#define SIGALRM		14
#define SIGTERM		15
#define SIGSTKFLT	16
#define SIGCHLD		17
#define SIGCONT		18
#define SIGSTOP		19
#define SIGTSTP		20
#define SIGTTIN		21
#define SIGTTOU		22
#define SIGURG		23
#define SIGXCPU		24
#define SIGXFSZ		25
#define SIGVTALRM	26
#define SIGPROF		27
#define SIGWINCH	28
#define SIGIO		29
#define SIGPOLL		SIGIO
/*
#define SIGLOST		29
*/
#define SIGPWR		30
#define SIGSYS		31
#define	SIGUNUSED	31

/* These should not be considered constants from userland.  */
#define SIGRTMIN	32
#ifndef SIGRTMAX
#define SIGRTMAX	_NSIG
#endif

#ifndef SIG_BLOCK
#define SIG_BLOCK          0	/* for blocking signals */
#endif
#ifndef SIG_UNBLOCK
#define SIG_UNBLOCK        1	/* for unblocking signals */
#endif
#ifndef SIG_SETMASK
#define SIG_SETMASK        2	/* for setting the signal mask */
#endif

typedef struct {
    unsigned long sig[_NSIG_WORDS];
} sigset_t;

typedef void __signalfn_t(int);
typedef __signalfn_t* __sighandler_t;

typedef void __restorefn_t(void);
typedef __restorefn_t* __sigrestore_t;

#define SIG_DFL	((__sighandler_t)0)	/* default signal handling */
#define SIG_IGN	((__sighandler_t)1)	/* ignore signal */
#define SIG_ERR	((__sighandler_t)-1)	/* error return from signal */

struct sigaction {
    __sighandler_t sa_handler;
    unsigned long sa_flags;
    __sigrestore_t sa_restorer;
    sigset_t sa_mask;		/* mask last for extensibility */
};

typedef __sighandler_t sighandler_t;

typedef struct jmp_buf_s {
    long long reg[8];
    sigset_t mask;
} jmp_buf[1];

typedef __SIZE_TYPE__ size_t;
typedef long int ssize_t;

#endif /* _SIGNAL_H */

#include "hw2.h"

int main(int argc, char* argv[]) {

    // argv[0] = this program
    // argv[1] = command
    // argv[2] = path to logger.so
    // argv[3] = output target (optional)

    if (argc == 3) {
        dup2(STDERR_FILENO, 3);
    }
    else if (argc == 4) {
        int fd = creat(argv[3], 0644);
        if (fd < 0) errquit("creat");
        if (fd != 3) dup2(fd, 3);
    }
    else {
        fprintf(stderr, "Something went wrong.\n");
        return -1;
    }

    char command[MAXLINE] = {};
    snprintf(command, MAXLINE, "LD_PRELOAD=%s %s", argv[2], argv[1]);

    execlp("bash", "bash", "-c", command, NULL);

    return 0;
}
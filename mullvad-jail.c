#define _GNU_SOURCE
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    if (geteuid() != 0) {
        fprintf(stderr, "Error: need to be root or have setuid\n");
        exit(1);
    }
    if (argc < 2) {
        fprintf(stderr, "Error: at least one argument needed\n");
        exit(1);
    }
    int fd = open("/var/run/netns/mullvad-jail", O_RDONLY);
    if ((fd == -1) || (setns(fd, CLONE_NEWNET) == -1)) {
        fprintf(stderr, "Error: could not set namespace\n");
        exit(1);
    }
    setuid(getuid());
    execvp(argv[1], &argv[1]);
}
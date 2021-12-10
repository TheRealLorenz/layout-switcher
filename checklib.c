#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

void check_malloc(void *ptr) {
    if (!ptr) {
        fprintf(stderr, "An error occurred while using malloc! ERROR: %s\n", strerror(errno));
        exit(1);
    }
}

void check_pid(int pid) {
    if (pid == -1) {
        fprintf(stderr, "An error occurred while trying to fork! ERROR: %s\n", strerror(errno));
        exit(1);
    }
}

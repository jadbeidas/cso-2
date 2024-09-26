#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int my_system(const char *command) {
    if (command == NULL) {
        // don't check for shell availability as per the requirement
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        // fork failed
        return -1;
    } else if (pid == 0) {
        // in child process, execute command
        execlp("/bin/sh", "sh", "-c", command, (char *)NULL);
        // if execlp fails, exit with failure code 127
        _exit(127);
    } else {
        // in parent process, wait for child to finish and return wait status
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            return -1; // error in waiting for the child
        }

        return status;
    }
}
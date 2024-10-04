#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

void check_internal(const char *msg, int okay) {
    if (okay == -1) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
}

char *getoutput(const char *command) {
    int pipefd[2];
    pid_t pid;
    char *output = NULL;
    size_t buf_size = 0;
    FILE *pipe_read_stream;

    // create the pipe
    check_internal("pipe", pipe(pipefd));

    // fork the process
    pid = fork();
    check_internal("fork", pid);

    if (pid == 0) {
        // child process
        close(pipefd[0]);  // close read end of the pipe
        check_internal("dup2", dup2(pipefd[1], STDOUT_FILENO));  // redirect stdout to the pipe
        close(pipefd[1]);  // close write end after duplicating

        execl("/bin/sh", "sh", "-c", command, (char *)NULL);
        _exit(127);  // should not reach here unless exec fails
    } else {
        // parent process
        close(pipefd[1]);  // close write end of the pipe

        // use fdopen to convert the file descriptor to a FILE*
        pipe_read_stream = fdopen(pipefd[0], "r");
        check_internal("fdopen", pipe_read_stream ? 0 : -1);

        // read the child's output using getdelim
        if (getdelim(&output, &buf_size, '\0', pipe_read_stream) == -1) {
            perror("getdelim");
            free(output);
            output = NULL;
        }

        close(pipefd[0]);  // close read end
        fclose(pipe_read_stream);  // close the FILE* stream

        // wait for the child to finish
        waitpid(pid, NULL, 0);
    }

    return output;
}

char *parallelgetoutput(int count, const char **argv_base) {
    int pipefd[2];
    pid_t pid;
    char *output = NULL;
    size_t buf_size = 0;
    FILE *pipe_read_stream;
    char index_str[12];  // buffer to hold the child index converted to string

    check_internal("pipe", pipe(pipefd));  // create a single pipe

    for (int i = 0; i < count; ++i) {
        pid = fork();
        check_internal("fork", pid);

        if (pid == 0) {
            // child process
            close(pipefd[0]);  // close read end of the pipe in child

            // convert the child index to a string
            snprintf(index_str, sizeof(index_str), "%d", i);

            // count the number of arguments in argv_base
            int arg_count = 0;
            while (argv_base[arg_count] != NULL) {
                arg_count++;
            }

            // build the new argument list for exec
            const char **new_argv = malloc((arg_count + 2) * sizeof(char *));
            if (!new_argv) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }

            for (int j = 0; j < arg_count; ++j) {
                new_argv[j] = argv_base[j];
            }
            new_argv[arg_count] = index_str;  // add the child index as the last argument
            new_argv[arg_count + 1] = NULL;

            check_internal("dup2", dup2(pipefd[1], STDOUT_FILENO));  // redirect stdout to pipe
            close(pipefd[1]);  // close write end after duplicating

            execv(new_argv[0], (char *const *)new_argv);  // execute the command
            _exit(127);  // should not reach here unless exec fails

            // free the allocated memory for new_argv in the child process (though _exit should handle it)
            free(new_argv);
        }
    }

    // parent process: Close write end of the pipe
    close(pipefd[1]);

    // use fdopen to convert the file descriptor to a FILE*
    pipe_read_stream = fdopen(pipefd[0], "r");
    check_internal("fdopen", pipe_read_stream ? 0 : -1);

    // read all child outputs
    if (getdelim(&output, &buf_size, '\0', pipe_read_stream) == -1) {
        perror("getdelim");
        free(output);
        output = NULL;
    }

    close(pipefd[0]);
    fclose(pipe_read_stream);

    // wait for all children to finish
    for (int i = 0; i < count; ++i) {
        waitpid(-1, NULL, 0);
    }

    return output;
}
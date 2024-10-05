#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

char *getoutput(const char *command) {
    int pipefd[2];
    pid_t pid;
    char *output = NULL;
    size_t buf_size = 0;
    FILE *pipe_read_stream;

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(127);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(127);
    }

    if (pid == 0) {
        close(pipefd[0]);
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(127);
        }
        close(pipefd[1]);

        execl("/bin/sh", "sh", "-c", command, (char *)NULL);
        _exit(127);
    } else {
        close(pipefd[1]);

        pipe_read_stream = fdopen(pipefd[0], "r");
        if (pipe_read_stream == NULL) {
            perror("fdopen");
            exit(127);
        }

        if (getdelim(&output, &buf_size, '\0', pipe_read_stream) == -1) {
            perror("getdelim");
            free(output);
            output = NULL;
        }

        close(pipefd[0]);
        fclose(pipe_read_stream);

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
    char index_str[12];

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(127);
    }

    for (int i = 0; i < count; ++i) {
        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(127);
        }

        if (pid == 0) {
            close(pipefd[0]);

            snprintf(index_str, sizeof(index_str), "%d", i);

            int arg_count = 0;
            while (argv_base[arg_count] != NULL) {
                arg_count++;
            }

            const char **new_argv = malloc((arg_count + 2) * sizeof(char *));
            if (!new_argv) {
                perror("malloc");
                exit(127);
            }

            for (int j = 0; j < arg_count; ++j) {
                new_argv[j] = argv_base[j];
            }
            new_argv[arg_count] = index_str;
            new_argv[arg_count + 1] = NULL;

            if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
                perror("dup2");
                exit(127);
            }
            close(pipefd[1]);

            execv(new_argv[0], (char *const *)new_argv);
            _exit(127);

            free(new_argv);
        }
    }

    close(pipefd[1]);

    pipe_read_stream = fdopen(pipefd[0], "r");
    if (pipe_read_stream == NULL) {
        perror("fdopen");
        exit(127);
    }

    if (getdelim(&output, &buf_size, '\0', pipe_read_stream) == -1) {
        perror("getdelim");
        free(output);
        output = NULL;
    }

    close(pipefd[0]);
    fclose(pipe_read_stream);

    for (int i = 0; i < count; ++i) {
        waitpid(-1, NULL, 0);
    }

    return output;
}
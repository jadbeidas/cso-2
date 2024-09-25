#define _XOPEN_SOURCE 700 // request all POSIX features, even with -std=c11
#include <fcntl.h>        // for O_CREAT, O_RDWR
#include <limits.h>       // for NAME_MAX
#include <stdlib.h>       // for EXIT_SUCCESS, NULL, abort
#include <stdio.h>        // for getline, perror
#include <sys/mman.h>     // for mmap, shm_open
#include <time.h>         // for nanosleep 
#include <unistd.h>       // for getpid

#include <signal.h>

pid_t other_pid = 0;

#define BOX_SIZE 4096
char *my_inbox;     // inbox of current process, set by setup_inboxes()
char *other_inbox;  // inbox of PID other_pid, set by setup_inboxes()

// bookkeeping for cleanup_inboxes()
char my_inbox_shm_open_name[NAME_MAX];
char other_inbox_shm_open_name[NAME_MAX];

/* open, creating if needed, an inbox shared memory region
   for the specified pid.

   store the shm_open filename used in filename, which must
   be NAME_MAX bytes long.
 */
char *setup_inbox_for(pid_t pid, char *filename) {
    snprintf(filename, NAME_MAX, "/%d-chat", pid);
    int fd = shm_open(filename, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        perror("shm_open");
        abort();
    }
    if (ftruncate(fd, BOX_SIZE) != 0) {
        perror("ftruncate");
        abort();
    }
    char *ptr = mmap(NULL, BOX_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == (char*) MAP_FAILED) {
        perror("mmap");
        abort();
    }
    return ptr;
}

void setup_inboxes() {
    my_inbox = setup_inbox_for(getpid(), my_inbox_shm_open_name);
    other_inbox = setup_inbox_for(other_pid, other_inbox_shm_open_name);
}

void cleanup_inboxes() {
    munmap(my_inbox, BOX_SIZE);
    munmap(other_inbox, BOX_SIZE);
    shm_unlink(my_inbox_shm_open_name);
}

void signal_handler(int pid)
{
    if(pid == SIGTERM)
    {
        cleanup_inboxes();
        exit(EXIT_SUCCESS);
    }
    else if(pid == SIGINT)
    {
        cleanup_inboxes();
        kill(other_pid, SIGTERM);
        exit(EXIT_SUCCESS);
    }
    else if(pid == SIGUSR1)
    {
        printf("%s", my_inbox);
        my_inbox[0] = '\0';
    }
}

int main(void) {
    printf("This process's ID: %ld\n", (long) getpid());
    char *line = NULL; size_t line_length = 0;
    do {
        printf("Enter other process ID: ");
        if (-1 == getline(&line, &line_length, stdin)) {
            perror("getline");
            abort();
        }
    } while ((other_pid = strtol(line, NULL, 10)) == 0);
    free(line);
    setup_inboxes();
    
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);

    while (1) {
        printf("Enter a message: ");
        
        if (fgets(other_inbox, BOX_SIZE, stdin) == NULL) {
            kill(other_pid, SIGTERM);
            break;
        }
        
        kill(other_pid, SIGUSR1);
        
        while (other_inbox[0] != '\0') {
            struct timespec ts = { .tv_sec = 0, .tv_nsec = 10000000 };
            nanosleep(&ts, NULL);
        }
    }

    cleanup_inboxes();
    return EXIT_SUCCESS;
}
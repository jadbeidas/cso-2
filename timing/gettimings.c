#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <linux/time.h>

#define NANOSECONDS_IN_SECOND 1000000000L

// helper function to calculate time difference
struct timespec diff(struct timespec start, struct timespec end) {
    struct timespec temp;
    if ((end.tv_nsec - start.tv_nsec) < 0) {
        temp.tv_sec = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = NANOSECONDS_IN_SECOND + end.tv_nsec - start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return temp;
}

// calculate overhead by measuring an empty clock_gettime call
struct timespec calculate_overhead() {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    clock_gettime(CLOCK_MONOTONIC, &end);
    return diff(start, end);
}

void empty_function() {}

// scenario 1: empty function call
void empty_function_time(struct timespec overhead) {
    struct timespec start, end, delta, total = {0, 0};

    for(int i = 0; i < 1000; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        empty_function(); // call empty function
        clock_gettime(CLOCK_MONOTONIC, &end);

        delta = diff(start, end);
        delta.tv_sec -= overhead.tv_sec;
        delta.tv_nsec -= overhead.tv_nsec;
        if (delta.tv_nsec < 0) {
            delta.tv_sec -= 1;
            delta.tv_nsec += NANOSECONDS_IN_SECOND;
        }

        total.tv_sec += delta.tv_sec;
        total.tv_nsec += delta.tv_nsec;
        if (total.tv_nsec >= NANOSECONDS_IN_SECOND) {
            total.tv_sec += 1;
            total.tv_nsec -= NANOSECONDS_IN_SECOND;
        }
    }
    
    total.tv_sec /= 1000;
    total.tv_nsec /= 1000;
    printf("Average empty function time: %ld.%09ld seconds\n", total.tv_sec, total.tv_nsec);
}

// scenario 2: getppid() system call
void getppid_time(struct timespec overhead) {
    struct timespec start, end, delta, total = {0, 0};

    for(int i = 0; i < 1000; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        getppid(); // system call
        clock_gettime(CLOCK_MONOTONIC, &end);

        delta = diff(start, end);
        delta.tv_sec -= overhead.tv_sec;
        delta.tv_nsec -= overhead.tv_nsec;
        if (delta.tv_nsec < 0) {
            delta.tv_sec -= 1;
            delta.tv_nsec += NANOSECONDS_IN_SECOND;
        }

        total.tv_sec += delta.tv_sec;
        total.tv_nsec += delta.tv_nsec;
        if (total.tv_nsec >= NANOSECONDS_IN_SECOND) {
            total.tv_sec += 1;
            total.tv_nsec -= NANOSECONDS_IN_SECOND;
        }
    }

    total.tv_sec /= 1000;
    total.tv_nsec /= 1000;
    printf("Average getppid() time: %ld.%09ld seconds\n", total.tv_sec, total.tv_nsec);
}

// scenario 3: system("/bin/true")
void system_true_time(struct timespec overhead) {
    struct timespec start, end, delta, total = {0, 0};

    for(int i = 0; i < 1000; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        system("/bin/true"); // run command
        clock_gettime(CLOCK_MONOTONIC, &end);

        delta = diff(start, end);
        delta.tv_sec -= overhead.tv_sec;
        delta.tv_nsec -= overhead.tv_nsec;
        if (delta.tv_nsec < 0) {
            delta.tv_sec -= 1;
            delta.tv_nsec += NANOSECONDS_IN_SECOND;
        }

        total.tv_sec += delta.tv_sec;
        total.tv_nsec += delta.tv_nsec;
        if (total.tv_nsec >= NANOSECONDS_IN_SECOND) {
            total.tv_sec += 1;
            total.tv_nsec -= NANOSECONDS_IN_SECOND;
        }
    }

    total.tv_sec /= 1000;
    total.tv_nsec /= 1000;
    printf("Average system(\"/bin/true\") time: %ld.%09ld seconds\n", total.tv_sec, total.tv_nsec);
}

// scenario 4: signal to self
void signal_self_time(struct timespec overhead) {
    struct timespec start, end, delta, total = {0, 0};

    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGUSR1, &sa, NULL);

    for(int i = 0; i < 1000; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        kill(getpid(), SIGUSR1); // send signal to self
        clock_gettime(CLOCK_MONOTONIC, &end);

        delta = diff(start, end);
        delta.tv_sec -= overhead.tv_sec;
        delta.tv_nsec -= overhead.tv_nsec;
        if (delta.tv_nsec < 0) {
            delta.tv_sec -= 1;
            delta.tv_nsec += NANOSECONDS_IN_SECOND;
        }

        total.tv_sec += delta.tv_sec;
        total.tv_nsec += delta.tv_nsec;
        if (total.tv_nsec >= NANOSECONDS_IN_SECOND) {
            total.tv_sec += 1;
            total.tv_nsec -= NANOSECONDS_IN_SECOND;
        }
    }

    total.tv_sec /= 1000;
    total.tv_nsec /= 1000;
    printf("Average signal to self time: %ld.%09ld seconds\n", total.tv_sec, total.tv_nsec);
}

// scenario 5: signal round-trip with another process
void signal_round_trip(int partner_pid, struct timespec overhead) {
    struct timespec start, end, delta, total = {0, 0};

    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGUSR1, &sa, NULL);

    for(int i = 0; i < 1000; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        kill(partner_pid, SIGUSR1); // send signal to partner process
        pause(); // wait for signal back
        clock_gettime(CLOCK_MONOTONIC, &end);

        delta = diff(start, end);
        delta.tv_sec -= overhead.tv_sec;
        delta.tv_nsec -= overhead.tv_nsec;
        if (delta.tv_nsec < 0) {
            delta.tv_sec -= 1;
            delta.tv_nsec += NANOSECONDS_IN_SECOND;
        }

        total.tv_sec += delta.tv_sec;
        total.tv_nsec += delta.tv_nsec;
        if (total.tv_nsec >= NANOSECONDS_IN_SECOND) {
            total.tv_sec += 1;
            total.tv_nsec -= NANOSECONDS_IN_SECOND;
        }
    }

    total.tv_sec /= 1000;
    total.tv_nsec /= 1000;
    printf("Average signal round-trip time: %ld.%09ld seconds\n", total.tv_sec, total.tv_nsec);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <scenario_number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int scenario = atoi(argv[1]);
    struct timespec overhead = calculate_overhead();

    switch (scenario) {
        case 1:
            empty_function_time(overhead);
            break;
        case 2:
            getppid_time(overhead);
            break;
        case 3:
            system_true_time(overhead);
            break;
        case 4:
            signal_self_time(overhead);
            break;
        case 5: {
            int partner_pid;
            printf("Enter partner PID: ");
            scanf("%d", &partner_pid);
            signal_round_trip(partner_pid, overhead);
            break;
        }
        case -1: {
            int my_pid = getpid();
            printf("My PID: %d\n", my_pid);
            int partner_pid;
            printf("Enter partner PID: ");
            scanf("%d", &partner_pid);
            while (1) {
                pause();
                kill(partner_pid, SIGUSR1); // send signal back
            }
            break;
        }
        default:
            fprintf(stderr, "Invalid scenario number.\n");
            exit(EXIT_FAILURE);
    }

    return 0;
}
#include "life.h"
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    LifeBoard *state1;
    LifeBoard *state2;
    int steps;
    int begin_row;
    int final_row;
    pthread_barrier_t *barrier;
} ThreadArgs;

void *task_executor(void *arg) {
    ThreadArgs *thread_args = (ThreadArgs *)arg;
    LifeBoard *state1 = thread_args -> state1;
    LifeBoard *state2 = thread_args -> state2;
    int steps = thread_args -> steps;
    int begin_row = thread_args -> begin_row;
    int final_row = thread_args -> final_row;
    pthread_barrier_t *barrier = thread_args -> barrier;

    int width = state1 -> width;

    for (int step = 0; step < steps; step += 1) {
        LifeBoard *curr_state;
        LifeBoard *next_state;

        if (steps % 2 == 0) {
            curr_state = state1;
            next_state = state2;
        } else {
            curr_state = state2;
            next_state = state1;
        }

        for (int y = begin_row; y < final_row; y += 1) {
            for (int x = 1; x < width - 1; x += 1) {
                int live_in_window = 0;

                for (int y_offset = -1; y_offset <= 1; y_offset+= 1) {
                    for (int x_offset = -1; x_offset <= 1; x_offset+= 1) {
                        int neighbor_x = y + x_offset;
                        int neighbor_y = x + y_offset;
                        if (LB_get(curr_state, neighbor_x, neighbor_y)) {
                            live_in_window += 1;
                        }
                    }
                }

                LifeCell current_cell = LB_get(curr_state, x, y);
                LifeCell next_cell;
                if (live_in_window == 3 || (live_in_window == 4 && current_cell)) {
                    next_cell = 1;
                } else {
                    next_cell = 0;
                }

                LB_set(next_state, x, y, next_cell);
            }
        }
        pthread_barrier_wait(barrier);
    }
    return NULL;
}

void simulate_life_parallel(int threads, LifeBoard *state, int steps) {
    LifeBoard *state1 = LB_clone(state);
    LifeBoard *state2 = LB_new(state -> width, state -> height);

    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, threads);

    pthread_t *thread_ids = malloc(threads * sizeof(pthread_t));
    ThreadArgs *thread_args = malloc(threads * sizeof(ThreadArgs));

    int total_rows = state -> height - 2;
    int rows_per_thread = total_rows / threads;
    int extra_rows = total_rows % threads;
    int current_row = 1;

    for (int i = 0; i < threads; ++i) {
        thread_args[i].state1 = state1;
        thread_args[i].state2 = state2;
        thread_args[i].steps = steps;
        thread_args[i].barrier = &barrier;

        int rows;
        if (i < extra_rows) {
            rows = rows_per_thread + 1;
        } else {
            rows = rows_per_thread;
        }

        thread_args[i].begin_row = current_row;
        thread_args[i].final_row = current_row + rows;
        current_row += rows;

        pthread_create(&thread_ids[i], NULL, task_executor, &thread_args[i]);
    }

    for (int i = 0; i < threads; ++i) {
        pthread_join(thread_ids[i], NULL);
    }

    LifeBoard *final_state;
    if (steps % 2 == 0) {
        final_state = state1;
    } else {
        final_state = state2;
    }
    memcpy(state->cells, final_state->cells, state->width * state->height * sizeof(LifeCell));

    pthread_barrier_destroy(&barrier);
    LB_del(state1);
    LB_del(state2);
    free(thread_ids);
    free(thread_args);
}

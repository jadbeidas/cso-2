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
    LifeBoard *state1 = thread_args->state1;
    LifeBoard *state2 = thread_args->state2;
    int steps = thread_args->steps;
    int begin_row = thread_args->begin_row;
    int final_row = thread_args->final_row;
    pthread_barrier_t *barrier = thread_args->barrier;

    int width = state1->width;

    for (int step = 0; step < steps; step += 1) {
        LifeBoard *curr_state = (step % 2 == 0) ? state1 : state2;
        LifeBoard *next_state = (step % 2 == 0) ? state2 : state1;

        for (int y = begin_row; y < final_row; y += 1) {
            for (int x = 1; x < width - 1; x += 1) {
                int live_in_window = 0;

                for (int y_offset = -1; y_offset <= 1; y_offset+= 1) {
                    for (int x_offset = -1; x_offset <= 1; x_offset+= 1) {
                        if (x_offset != 0 || y_offset != 0) {
                            live_in_window += LB_get(curr_state, x + x_offset, y + y_offset);
                        }
                    }
                }

                LifeCell current_cell = LB_get(curr_state, x, y);
                LifeCell next_cell = (live_in_window == 3 || (current_cell && live_in_window == 2)) ? 1 : 0;
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

    pthread_t thread_ids[threads];
    ThreadArgs thread_args[threads];

    int total_rows = state -> height - 2;
    int rows_per_thread = total_rows / threads;
    int extra_rows = total_rows % threads;
    int current_row = 1;

    for (int i = 0; i < threads; ++i) {
        thread_args[i].state1 = state1;
        thread_args[i].state2 = state2;
        thread_args[i].steps = steps;
        thread_args[i].barrier = &barrier;

        int rows = rows_per_thread + (i < extra_rows ? 1 : 0);

        thread_args[i].begin_row = current_row;
        thread_args[i].final_row = current_row + rows;
        current_row += rows;

        pthread_create(&thread_ids[i], NULL, task_executor, &thread_args[i]);
    }

    for (int i = 0; i < threads; ++i) {
        pthread_join(thread_ids[i], NULL);
    }

    LifeBoard *final_state = (steps % 2 == 0) ? state1 : state2;
    memcpy(state->cells, final_state->cells, state->width * state->height * sizeof(LifeCell));


    pthread_barrier_destroy(&barrier);
    LB_del(state1);
    LB_del(state2);
}

#include "say.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    int target = 2501;
    if (argc > 1) {
        long a = strtol(argv[1], NULL, 10);
        if (a > 0) target = a;
    }
    int last_guess = 0x80000000;
    for(int i=0; i<10; i+=1) {
        printf("Guess a number: ");
        int guess = -1;
        for(;;) {
            int res = scanf("%d", &guess);
            if (res == EOF) { puts("End of input found; terminating early..."); return 1; }
            if (res == 1 && guess > 0) break;
            printf("That was not a positive integer; try again: ");
        }
        if (target == guess) {
            hooray(10-i);
            return 0;
        }
        int diff = abs(target-guess) - abs(target-last_guess);
        last_guess = guess;
        if (diff < 0) { // better
            hip();
        } else if (diff > 0) {
            weep(diff > 10 ? 10 : diff);
        } else {
            accept();
        }
    }
    reject();
}

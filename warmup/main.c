#include "split.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_SIZE 4000

int main(int argc, char *argv[]) {
    char *sep;

    if (argc >= 2) {
        size_t sep_length = 0;

        // iterate through every string in args to find length of sep
        for (int i = 1; i < argc; i++) {
            sep_length += strlen(argv[i]);
        }

        // allocate space for sep and null terminator
        sep = malloc(sep_length + 1);
        if (!sep) {
            perror("malloc");
            return EXIT_FAILURE;
        }

        // concatenate all string args into one string
        sep[0] = '\0';
        for (int i = 1; i < argc; i++) {
            strcat(sep, argv[i]);
        }
    }
    else { // default case no args
        sep = " \t";
    }

    char input[MAX_INPUT_SIZE];
    while (fgets(input, sizeof(input), stdin)) {
        input[strcspn(input, "\n")] = '\0';

        // condition to end the program
        if (strcmp(input, ".") == 0) {
            break;
        }

        int word_count = 0;
        char **words = string_split(input, sep, &word_count);

        // iterate through and print words
        if (words) {
            printf("[");
            for (int i = 0; i < word_count; i++) {
                if (i > 0) {
                    printf("][");
                }
                printf("%s", words[i]);
                free(words[i]);
            }
            printf("]\n");
            free(words);
        }
    }

    if (argc > 1) {
        free(sep);
    }

    return EXIT_SUCCESS;
}
#include "split.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **string_split(const char *input, const char *sep, int *num_words) {
    *num_words = 0;
    if (!input || !sep) return NULL;

    //size_t input_len = strlen(input);
    char **words = NULL; // array of split words
    const char *cur_word = input; // start tracking current word/position in input

    // leading sep case, add empty string
    if (strchr(sep, *cur_word)) {
        words = realloc(words, (*num_words + 1) * sizeof(char *));
        if (!words) return NULL;

        words[*num_words] = strdup("");
        (*num_words)++;
    }

    while (*cur_word) {
        size_t leading_sep = strspn(cur_word, sep); // find the start of next word
        cur_word += leading_sep;

        size_t word_len = strcspn(cur_word, sep); // find the end of the word

        if (word_len > 0) { // check if theres either a leading sep char or a non empty word
            char *word = malloc(word_len + 1);
            if (!word) {
                for (int i = 0; i < *num_words; i++) {
                    free(words[i]);
                }
                free(words);
                return NULL;
            }

            memcpy(word, cur_word, word_len);
            word[word_len] = '\0';

            char **new_words = realloc(words, (*num_words + 1) * sizeof(char *)); // create space for an additional word
            if (!new_words) {
                free(word);
                for (int i = 0; i < *num_words; i++) {
                    free(words[i]);
                }
                free(words);
                return NULL;
            }
            words = new_words; // update pointer to newly resized array
            words[*num_words] = word; // pointer to newly allocated word

            (*num_words)++;

            cur_word += word_len; // move current pointer forward by length of word
        }
    }

    if (*(cur_word - 1) && strchr(sep, *(cur_word - 1))) { // checking the last character being a sep delimiter case
        char *word = malloc(1);
        if (!word) {
            for (int i = 0; i < *num_words; i++) {
                free(words[i]);
            }
            free(words);
            return NULL;
        }
        word[0] = '\0';

        char **new_words = realloc(words, (*num_words +1) * sizeof(char *));
        if (!new_words) {
            free(word);
            for( int i = 0; i < *num_words; i++) {
                free(words[i]);
            }
            free(words);
            return NULL;
        }
        words = new_words;
        words[*num_words] = word;
        (*num_words)++;
    }

    return words;
}
        
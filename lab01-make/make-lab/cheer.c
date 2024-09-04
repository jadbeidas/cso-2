#include <stdio.h>
#include "say.h"
void hip() {
    puts("Yay!");
}
void hooray(int rep) {
    for (int i=0; i<rep; i+=1)
        printf("Hip ");
    puts("Hooray!");
}
// bruh
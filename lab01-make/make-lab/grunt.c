#include <stdio.h>
#include "say.h"
void accept() {
    puts("Mm-hm");
}
void reject() {
    puts("Uh-uh");
}
void weep(int rep) {
    printf("Woe");
    for (int i=1; i<rep; i+=1)
        printf(" woe");
    puts(" is me!");
}

#include <stdio.h>
#include <stdlib.h>

char *getoutput(const char *command);
char *parallelgetoutput(int count, const char **argv_base);

int main() {
    // Test getoutput
    printf("Hi!\n");
    char *output = getoutput("echo 1 2 3; sleep 2; echo 5 5");
    printf("Text: [[[%s]]]\n", output);
    free(output);
    printf("Bye!\n");

    // Test parallelgetoutput
    const char *argv_base[] = {"/bin/echo", "running", NULL};
    output = parallelgetoutput(2, argv_base);
    printf("Text: [%s]\n", output);
    free(output);

    return 0;
}
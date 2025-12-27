#include <sys/types.h>
#include <sys/wait.h>

#include <string.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>

#define MAX_LINE 80

void s_sh_loop(void)
{
    // To-Do
    // Create a sequencitial mode & parallel mode

    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line   = s_sh_read_line();
        args   = s_sh_split_line(line);
        status = s_sh_execute(args);

        free(line);
        free(args);
    } while (status);

}

int main(int argc, char **argv)
{
    // To-Do
    // Add Initialization
    // Add Calendar/Welcome

    s_sh_loop(); // AKA Simple Sh... Later will change (maybe never)

    return EXIT_SUCCESS;
}
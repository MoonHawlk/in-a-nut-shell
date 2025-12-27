#include <sys/types.h>
#include <sys/wait.h>

#include <string.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>

#define LINE_BUFFER_SIZE 1024 // Can be more, however, consider that when creating a history command, this may take storage and could cause leakage 
#define TRUE 1
#define FALSE 0 

char *s_sh_read_line(void)
{
    int buffersize = LINE_BUFFER_SIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * buffersize);
    int element;

    if (!buffer)
    {
        fprintf(stderr, "ssh: Allocation Error");
        exit(EXIT_FAILURE);
    }

    // We "check" of exists the next element.
    // If yes, add to the array and go to the next
    // Else, stops and return
    while(TRUE) {

        element = getchar();

        if (element == EOF || element == '\n')
        {
            buffer[position] = '\0';
            return buffer;

        } else {
            buffer[position] = element;
        }
        position++;

        // Check if we exceeded the buffer size
        // If yes, we try to realocate.
        if(position >= buffersize) {
            buffersize += LINE_BUFFER_SIZE;
            buffer = realloc(buffer, buffersize);
            if (!buffer) {
                fprintf(stderr, "ssh: Allocation Error");
                exit(EXIT_FAILURE);
            }
        }

    }
}


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
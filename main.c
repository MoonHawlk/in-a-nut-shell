/*****************************************************************************

  @file         main.c

  @author       Filipe Moreno

  @date         Saturday,  27 December 2025

  @brief        fmsh (Filipe Moreno Shell)

  @todo         1 - Create a history command (maybe !! can solve that)
                2 - Make the shell read more than one command using ;
                3 - Create a parallel mode
                4 - Create Redirects
                5 - Create Pipepes

*******************************************************************************/

#include <sys/types.h>
#include <sys/wait.h>

#include <string.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>

#define LINE_BUFFER_SIZE 1024 // Can be more, however, consider that when creating a history command, this may take storage and could cause leakage 
#define TRUE 1
#define FALSE 0 

char *fmsh_read_line(void)
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


void fmsh_loop(void)
{
    // To-Do
    // Create a sequencitial mode & parallel mode

    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line   = fmsh_read_line();
        args   = fmsh_split_line(line);
        status = fmsh_execute(args);

        free(line);
        free(args);
    } while (status);

}

int main(int argc, char **argv)
{
    // To-Do
    // Add Initialization
    // Add Calendar/Welcome

    fmsh_loop(); // AKA Filipe Moreno Shell... I've changed it :D!!

    return EXIT_SUCCESS;
}
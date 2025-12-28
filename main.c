/**************************************************************************************************

  @file         main.c

  @author       Filipe Moreno

  @date         Saturday,  27 December 2025

  @brief        fmsh (Filipe Moreno Shell)

  @todo         1 - Create a history command (maybe !! can solve that)
                2 - Make the shell read more than one command using ;
                3 - Create a parallel mode
                4 - Create Redirects
                5 - Create Pipepes

  @cite         Code based ,inspired by and built upon the knowledge shared by Stephen Brennan
                (https://brennan.io/2015/01/16/write-a-shell-in-c/)

                Credits to him for creating such an excellent tutorial, which allowed me to quickly 
                improve my previous implementation and reminded me of the old C days back in college.

/**************************************************************************************************/

#include <sys/types.h>
#include <sys/wait.h>

#include <string.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>

#define LINE_BUFFER_SIZE 1024 // Can be more, however, consider that when creating a history command, this may take storage and could cause leakage 
#define FMSH_TOK_BUFFER_SIZE 64
#define FMSH_TOK_DELIMITER " \t\r\n\a"
#define TRUE 1
#define FALSE 0 



/*
  Function Declarations for builtin shell commands:
 */
int fmsh_cd(char **args);
int fmsh_help(char **args);
int fmsh_exit(char **args);


int fmsh_exit(char **args)
{
  return 0;
}

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &fmsh_cd,
  &fmsh_help,
  &fmsh_exit
};

int fmsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/
int fmsh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "fmsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("fmsh");
    }
  }
  return 1;
}

int fmsh_help(char **args)
{
  int i;
  printf("Filipe Moreno's fmsh\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < fmsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}


int fmsh_launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("fmsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("fmsh");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

void allocation_error_callback(void)
{
    fprintf(stderr, "fmsh: Allocation Error\n");
    exit(EXIT_FAILURE);
}

char **fmsh_split_line(char *line)
{
    int buffer_size = FMSH_TOK_BUFFER_SIZE, position = 0;
    char **tokens = malloc(buffer_size * sizeof(char*));
    char *token;

    if (!tokens) {
        allocation_error_callback();
    }

    token = strtok(line, FMSH_TOK_DELIMITER);
    while (token != NULL) 
    {
        tokens[position] = token;
        position++;

        if (position >= buffer_size)
        {
            buffer_size += FMSH_TOK_BUFFER_SIZE;
            tokens = realloc(tokens, buffer_size * sizeof(char*));

            if (!tokens) {
                allocation_error_callback();
            }
        }
        token = strtok(NULL, FMSH_TOK_DELIMITER);
    }
    tokens[position] = NULL;
    return tokens;
}
char *fmsh_read_line(void)
{
    int buffersize = LINE_BUFFER_SIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * buffersize);
    int element;

    if (!buffer)
    {
        allocation_error_callback();
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
                allocation_error_callback();
            }
        }

    }
}

int fmsh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < fmsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return fmsh_launch(args);
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
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

**************************************************************************************************/

#include <sys/types.h>
#include <sys/wait.h>

#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define LINE_BUFFER_SIZE 1024
#define FMSH_TOK_BUFFER_SIZE 64
#define FMSH_TOK_DELIMITER " \t\r\n\a"

/*
  Function Declarations for builtin shell commands:
 */
int fmsh_cd(char **args);
int fmsh_help(char **args);
int fmsh_exit(char **args);

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

int fmsh_num_builtins(void)
{
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
  (void)args;
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

/*
  exit builtin
  Supports optional exit code (e.g. exit 2)
*/
int fmsh_exit(char **args)
{
  int exit_code = EXIT_SUCCESS;

  if (args[1] != NULL) {
    exit_code = atoi(args[1]);
  }

  exit(exit_code);
}

/*
  Launch external programs
*/
int fmsh_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("fmsh");
    }
    _exit(EXIT_FAILURE);

  } else if (pid < 0) {
    perror("fmsh");

  } else {
    do {
      pid_t wpid = waitpid(pid, &status, 0);
      if (wpid == -1 && errno != EINTR) {
        perror("fmsh");
        break;
      }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

void allocation_error_callback(void)
{
  fprintf(stderr, "fmsh: Allocation Error\n");
  exit(EXIT_FAILURE);
}

/*
  ISO C replacement for strdup (portable, safe)
*/
char *fmsh_strdup(const char *src)
{
  size_t len = strlen(src) + 1;
  char *dst = malloc(len);

  if (!dst) {
    allocation_error_callback();
  }

  memcpy(dst, src, len);
  return dst;
}

/*
  Split line into tokens.
  Tokens are now owned by the caller,
  making this safe for future features like history and pipes.
*/
char **fmsh_split_line(char *line)
{
  int buffer_size = FMSH_TOK_BUFFER_SIZE;
  int position = 0;

  char **tokens = malloc(buffer_size * sizeof(char *));
  char *token;

  if (!tokens) {
    allocation_error_callback();
  }

  token = strtok(line, FMSH_TOK_DELIMITER);
  while (token != NULL) {

    tokens[position++] = fmsh_strdup(token);

    if (position >= buffer_size) {
      buffer_size *= 2;
      tokens = realloc(tokens, buffer_size * sizeof(char *));
      if (!tokens) {
        allocation_error_callback();
      }
    }

    token = strtok(NULL, FMSH_TOK_DELIMITER);
  }

  tokens[position] = NULL;
  return tokens;
}


/*
  Read a full line from stdin.
  Exits shell cleanly on EOF (Ctrl+D).
*/
char *fmsh_read_line(void)
{
  int buffersize = LINE_BUFFER_SIZE;
  int position = 0;
  char *buffer = malloc(buffersize);
  int element;

  if (!buffer) {
    allocation_error_callback();
  }

  while (true) {

    element = getchar();

    if (element == EOF) {
      free(buffer);
      printf("\n");
      exit(EXIT_SUCCESS);
    }

    if (element == '\n') {
      buffer[position] = '\0';
      return buffer;
    }

    buffer[position++] = (char)element;

    if (position >= buffersize) {
      buffersize *= 2;
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
    return 1;
  }

  for (i = 0; i < fmsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return fmsh_launch(args);
}

void fmsh_free_args(char **args)
{
  int i;
  for (i = 0; args[i] != NULL; i++) {
    free(args[i]);
  }
  free(args);
}

void fmsh_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("[>] ");
    line = fmsh_read_line();
    args = fmsh_split_line(line);
    status = fmsh_execute(args);

    free(line);
    fmsh_free_args(args);

  } while (status);
}

int main(int argc, char **argv)
{
  (void)argc;
  (void)argv;

  fmsh_loop();

  return EXIT_SUCCESS;
}

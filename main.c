/***********************************************************************************************************************************

  @file         main.c

  @author       Filipe Moreno

  @date         Saturday,  27 December 2025

  @brief        fmsh (Filipe Moreno Shell)

  @todo         Simple
                
                1 - Add Initialization options (Such as name of user and show time of commands). (Done - 28 December 2025)
                2 - Add Calendar/Welcome menssage when start. (Done - 28 December 2025) 

                Complex
                1 - Create a history command (maybe !! can solve that).
                2 - Make the shell read more than one command using ; .
                3 - Create a parallel mode.
                4 - Create Redirects.
                5 - Create Pipepes.

  @cite         Code based ,inspired by and built upon the knowledge shared by Stephen Brennan
                (https://brennan.io/2015/01/16/write-a-shell-in-c/)

                Credits to him for creating such an excellent tutorial, which allowed me to quickly 
                improve my previous implementation and reminded me of the old C days back in college.

***********************************************************************************************************************************/

#include <sys/types.h>
#include <sys/wait.h>

#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#define LINE_BUFFER_SIZE 1024
#define FMSH_TOK_BUFFER_SIZE 64
#define FMSH_TOK_DELIMITER " \t\r\n\a"

/****************************
   Builtin declarations
****************************/
int fmsh_cd(char **args);
int fmsh_help(char **args);
int fmsh_exit(char **args);

/****************************
   Configuration
****************************/
typedef struct {
  char user_name[64];
  bool show_time;
} fmsh_config_t;

static fmsh_config_t fmsh_config = {
  .user_name = "",
  .show_time = false
};

/****************************
   Builtin table
****************************/
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[])(char **) = {
  &fmsh_cd,
  &fmsh_help,
  &fmsh_exit
};

int fmsh_num_builtins(void)
{
  return sizeof(builtin_str) / sizeof(char *);
}

/****************************
   Builtins
****************************/
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

int fmsh_exit(char **args)
{
  int exit_code = EXIT_SUCCESS;

  if (args[1] != NULL) {
    exit_code = atoi(args[1]);
  }

  exit(exit_code);
}

/****************************
   Config loading
****************************/
void fmsh_load_config(const char *path)
{
  FILE *file = fopen(path, "r");
  char line[128];

  if (!file) {
    return; /* optional */
  }

  while (fgets(line, sizeof(line), file)) {

    if (line[0] == '#' || line[0] == '\n') {
      continue;
    }

    char *key = strtok(line, "=");
    char *value = strtok(NULL, "\n");

    if (!key || !value) {
      continue;
    }

    if (strcmp(key, "USER_NAME") == 0) {
      strncpy(fmsh_config.user_name, value,
              sizeof(fmsh_config.user_name) - 1);
      fmsh_config.user_name[sizeof(fmsh_config.user_name) - 1] = '\0';
    }
    else if (strcmp(key, "SHOW_TIME") == 0) {
      fmsh_config.show_time = (atoi(value) != 0);
    }
  }

  fclose(file);
}

/****************************
   User name normalization
****************************/
void fmsh_init_user_name(void)
{
  const char *env_user;

  if (fmsh_config.user_name[0] != '\0') {
    return;
  }

  env_user = getenv("USER");
  if (env_user && env_user[0] != '\0') {
    strncpy(fmsh_config.user_name, env_user,
            sizeof(fmsh_config.user_name) - 1);
    fmsh_config.user_name[sizeof(fmsh_config.user_name) - 1] = '\0';
    return;
  }

  strcpy(fmsh_config.user_name, "fmsh");
}

/****************************
   Welcome / Calendar
****************************/
void fmsh_print_welcome(void)
{
  time_t now = time(NULL);
  struct tm *tm_info = localtime(&now);

  printf("========================================\n");
  printf(" Welcome to fmsh\n");
  printf(" User: %s\n", fmsh_config.user_name);
  printf(" Date: %02d/%02d/%04d\n",
         tm_info->tm_mday,
         tm_info->tm_mon + 1,
         tm_info->tm_year + 1900);
  printf("========================================\n");
}

/****************************
   Process launching
****************************/
int fmsh_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();

  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      perror("fmsh");
    }
    _exit(EXIT_FAILURE);

  } else if (pid < 0) {
    perror("fmsh");

  } else {
    do {
      if (waitpid(pid, &status, 0) == -1 && errno != EINTR) {
        perror("fmsh");
        break;
      }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

/****************************
   Memory helpers
****************************/
void allocation_error_callback(void)
{
  fprintf(stderr, "fmsh: Allocation Error\n");
  exit(EXIT_FAILURE);
}

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

/****************************
   Parsing
****************************/
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

/**************************** 
   Input
****************************/
char *fmsh_read_line(void)
{
  int buffersize = LINE_BUFFER_SIZE;
  int position = 0;
  char *buffer = malloc(buffersize);
  int c;

  if (!buffer) {
    allocation_error_callback();
  }

  while (true) {

    c = getchar();

    if (c == EOF) {
      free(buffer);
      printf("\n");
      exit(EXIT_SUCCESS);
    }

    if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    }

    buffer[position++] = (char)c;

    if (position >= buffersize) {
      buffersize *= 2;
      buffer = realloc(buffer, buffersize);
      if (!buffer) {
        allocation_error_callback();
      }
    }
  }
}

/**************************** 
   Execution
****************************/
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
  for (int i = 0; args[i] != NULL; i++) {
    free(args[i]);
  }
  free(args);
}

/****************************    
    Main loop
****************************/
void fmsh_loop(void)
{
  char *line;
  char **args;
  int status;

  do {

    if (fmsh_config.show_time) {
      time_t now = time(NULL);
      struct tm *tm_info = localtime(&now);

      printf("[%02d:%02d:%02d %s]> ",
             tm_info->tm_hour,
             tm_info->tm_min,
             tm_info->tm_sec,
             fmsh_config.user_name);
    } else {
      printf("[%s]> ", fmsh_config.user_name);
    }

    line = fmsh_read_line();
    args = fmsh_split_line(line);
    status = fmsh_execute(args);

    free(line);
    fmsh_free_args(args);

  } while (status);
}

/**************************** 
   Entry point
*****************************/
int main(int argc, char **argv)
{
  (void)argc;
  (void)argv;

  fmsh_load_config("fmsh_configs.fmsh");
  fmsh_init_user_name();
  fmsh_print_welcome();
  fmsh_loop();

  return EXIT_SUCCESS;
}

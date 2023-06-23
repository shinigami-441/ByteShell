
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <limits.h>
#define BUFSIZE 1024
#define TOK_BUFSIZE 64
#define MAX_SIZE 100
#define TOK_DELIM " \t\r\n\a"
#define MAX_HISTORY_SIZE 100
#define MAX_COMMAND_LENGTH 1024

int sz;
char *alias[MAX_SIZE][2];
int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);
int shell_alias(char **args);
int shell_history(char **args);
int shell_touch(char **args);
int shell_echo(char **args);
char history[MAX_HISTORY_SIZE][MAX_COMMAND_LENGTH];
int history_count = 0;
char *builtin_str[] = {
    "cd",
    "help",
    "exit",
    "alias",
    "history",
    "touch",
    "echo"};

int (*builtin_func[])(char **) = {
    &shell_cd,
    &shell_help,
    &shell_exit,
    &shell_alias,
    &shell_history,
    &shell_touch,
    &shell_echo

};

int num_builtins()
{
  return sizeof(builtin_str) / sizeof(char *);
}
void print_integer(int value)
{

  if (value < 0)
  {
    putchar('-');
    value = -1 * value;
  }

  if (value / 10 != 0)
  {
    print_integer(value / 10);
  }

  putchar('0' + (value % 10));
}

void print_double(double value)
{

  if (value < 0)
  {
    putchar('-');
    value = -1 * value;
  }

  int integer_part = (int)value;
  print_integer(integer_part);

  putchar('.');

  int decimal_part = (int)((value - integer_part) * 100);
  print_integer(decimal_part);
}

void print_string(char *c)
{
  while (*c != '\0')
  {
    putchar(*c);
    c++;
  }
}
void shell_printf(const char *format, ...)
{
  va_list args;
  va_start(args, format);

  while (*format != '\0')
  {
    if (*format == '%')
    {
      format++;

      switch (*format)
      {
      case 'd':
      {
        int value = va_arg(args, int);
        print_integer(value);
        break;
      }
      case 'f':
      {
        double value = va_arg(args, double);
        print_double(value);
        break;
      }
      case 's':
      {
        char *value = va_arg(args, char *);
        print_string(value);
        break;
      }
      default:
        putchar(*format);
        break;
      }
    }
    else
    {
      putchar(*format);
    }

    format++;
  }

  va_end(args);
}
int shell_echo(char** args){
    while(*++args){
    shell_printf("%s", *args);
    if(args[1])shell_printf(" ");
  }
  shell_printf("\n");
  return 1;
}
void add_to_history(const char *command)
{
  if (history_count == MAX_HISTORY_SIZE)
  {

    for (int i = 1; i < MAX_HISTORY_SIZE; ++i)
    {
      strcpy(history[i - 1], history[i]);
    }
    strcpy(history[MAX_HISTORY_SIZE - 1], command);
  }
  else
  {
    strcpy(history[history_count], command);
    ++history_count;
  }
}
int shell_history(char **args)
{
  for (int i = 0; i < history_count; ++i)
  {
    shell_printf("%d. %s\n", i + 1, history[i]);
  }
  return 1;
}
int shell_touch(char **args)
{
  const char *filename = args[1];
  int fd = open(filename, O_CREAT | S_IRUSR | S_IWUSR);

  if (fd == -1)
  {
    perror("Unable to touch file");
    return 0;
  }

  close(fd);
  return 1;
}
int shell_cd(char **args)
{
  if (args[1] == NULL)
  {
    fprintf(stderr, "coconut: expected directory name");
  }
  else
  {
    if (chdir(args[1]) != 0)
    {
      perror("coconut: error changing to specified directory");
    }
  }
  return 1;
}

int shell_help(char **args)
{
  int i;
  printf("***************************************************Coconut Shell*******************************************************************************\n");
  printf("Look up the builtin commands here.\n");

  for (i = 0; i < num_builtins(); i++)
  {
    printf("  %s\n", builtin_str[i]);
  }

  return 1;
}

int shell_exit(char **args)
{
  return 0;
}
int shell_alias(char **args)
{
  if (sz >= MAX_SIZE)
  {
    perror("coconut: ERROR Maximum number of available alias reached.");
    return 1;
  }
  const char *con = "-p";
  if (strcmp(con, args[1]) == 0)
  {
    for (int j = 0; j < sz; j++)
    {
      shell_printf("%s->%s\n", alias[j][0], alias[j][1]);
    }
    return 1;
  }
  int i = 0;
  while (args[1][i] != '=' && i < strlen(args[1]))
  {

    i++;
  }
  if (args[1][i] != '=')
  {
    perror("coconut: ERROR not valid alias arguement");
    return 1;
  }
  char *s = malloc(sizeof(char) * (i + 2));
  i = 0;
  while (args[1][i] != '=')
  {
    s[i] = args[1][i];
    i++;
  }
  s[i] = '\0';

  i++;

  int j = i + 1;
  int counter = 0;

  if (args[1][i] != '\'')
  {
    perror("coconut : ERROR Not valid command for Alias");
    return 1;
  }
  i++;

  while (args[1][i] != '\'' && args[1][i] != '\0' && args[1][i] != EOF && args[1][i] != '\n')
  {

    i++;
    counter++;
  }
  char *t = malloc(sizeof(char) * (counter + 1));
  i = 0;
  while (args[1][j] != '\'' && args[1][j] != '\0' && args[1][j] != EOF && args[1][j] != '\n')
  {
    t[i] = args[1][j];
    j++;
    i++;
  }
  t[i] = '\0';

  int index = sz;
  alias[index][0] = s;
  alias[index][1] = t;
  sz++;
  return 1;
}

int shell_launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0)
  {

    if (execvp(args[0], args) == -1)
    {
      perror("coconut: ERROR can't execute the command.");
    }
    exit(EXIT_FAILURE);
  }
  else if (pid < 0)
  {

    perror("coconut: ERROR can't fork");
  }
  else
  {

    do
    {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int shell_execute(char **args)
{
  int i;

  if (args[0] == NULL)
  {

    return 1;
  }
  add_to_history(args[0]);
  for (i = 0; i < num_builtins(); i++)
  {
    if (strcmp(args[0], builtin_str[i]) == 0)
    {
      return (*builtin_func[i])(args);
    }
  }
  for (int i = 0; i < sz; i++)
  {

    if (strcmp(alias[i][0], args[0]) == 0)
    {

      strcpy(args[0], alias[i][1]);

      return shell_execute(args);
    }
  }

  return shell_launch(args);
}

char *shell_read_line(void)
{
  int bufsize = BUFSIZE;
  int i = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  char c;
  int position = 0;
  if (!buffer)
  {
    fprintf(stderr, "coconut: ERROR Memory allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1)
  {

    c = getchar();

    if (c == EOF || c == '\n')
    {
      buffer[position] = '\0';
      return buffer;
    }
    else
    {
      buffer[position] = c;
    }
    position++;

    if (position >= bufsize)
    {
      bufsize += BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer)
      {
        fprintf(stderr, "coconut : ERROR allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

char **shell_split_line(char *line)
{
  int bufsize = TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token;

  if (!tokens)
  {
    fprintf(stderr, "coconut: ERROR allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, TOK_DELIM);
  while (token != NULL)
  {
    tokens[position] = token;
    position++;

    if (position >= bufsize)
    {
      bufsize += TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char *));
      if (!tokens)
      {
        fprintf(stderr, "coconut: ERROR allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

void loop(void)
{
  char *line;
  char **args;
  int status;

  do
  {
    printf(":) ");
    line = shell_read_line();
    args = shell_split_line(line);
    status = shell_execute(args);

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv)
{
  int sz = 0;
  loop();

  return EXIT_SUCCESS;
}

// Initial code for shell along with header files which maybe required for
// reference
#include <dirent.h> // for ls
#include <errno.h>
#include <fcntl.h> // used for open
#include <limits.h>
#include <stdbool.h> // for while loop
#include <stdio.h>
#include <stdlib.h> // used to execute subprocess and commands
#include <string.h>
#include <sys/stat.h>  // for mkdir
#include <sys/types.h> //pid_T
#include <sys/wait.h>
#include <unistd.h> // used for exit, getcwd,read, write, exec
#define TOKEN_BUFSIZE 1024
#define MAX_LINE_LENGTH 256
// I implement this to assume a certain size for input
//  read command line for the input line in shell
#define clear() printf("\033[H\033[J")

#define SUCCESS_MSG "\033[32mSuccess\033[0m\n"
#define ERROR_MSG "\033[31mError\033[0m\n"

// Define the command functions

int nate_cd();
int nate_exit(char **args);
void nate_help();
int nate_mkdir(char *directory_name);
int nate_rmdir(char *directory_name);
int list_directory(const char *dirname);
int nate_exec(char **args);
char **nate_parse(char *my_line);
void nate_loop(void);
char *nate_read_line(void);
void cat(char *filename);

// filename will be the makefile
void cat(char *filename) {
  FILE *fp;
  char c;

  fp = fopen(filename, "r"); /// some type of file name
  while ((c = fgetc(fp)) != EOF) {
    printf("%c", c);
  }
  fclose(fp);
}

void grep(const char *pattern, const char *filename) {
  char line[MAX_LINE_LENGTH];
  FILE *file = fopen(filename, "r"); // same type of filename
  if (file == NULL) {
    fprintf(stderr, "Error: could not open file %s\n", filename);
    exit(EXIT_FAILURE);
  }
  while (fgets(line, MAX_LINE_LENGTH, file)) {
    if (strstr(line, pattern)) {
      printf("%s", line);
    }
  }
  fclose(file);
}

void touch(const char *filename) {
  int fd =
      open(filename, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (fd == -1) {
    perror("Error: could not create file");
    exit(EXIT_FAILURE);
  }
  close(fd);
}

int nate_cd() {
  char path[100];
  printf("Enter directory path: ");
  fgets(path, 100, stdin);
  // remove newline character from the end of the path
  if (path[strlen(path) - 1] == '\n') {
    path[strlen(path) - 1] = '\0';
  }
  int ret = chdir(path);
  if (ret == -1) {
    perror("cd");
  } else {
    printf("Directory changed successfully\n");
  }
  return ret;
}

void greeting() {
  // clear();
  printf("\n\n\n\n******************"
         "************************");
  printf("\n\n\n\t****####Nathan A's Shell 187009702****");
  printf("\n\n\t-It works.....I think");
  printf("\n\n\n\n*******************"
         "***********************");
  char *username = getenv("USER");
  printf("\n\n\nUSER is: @%s", username);
  printf("\n");
  sleep(1);
  clear();
}

// this is for help
void nate_help() {
  puts("Nate Shell\n"
       "Enter a command and arguments, then press enter.\n"
       "The following commands are available:\n"
       "cd <directory>: Change the current working directory\n"
       "exit: Exit the shell\n"
       "help: Print this help message\n"
       "mkdir <directory>: Create a new directory\n"
       "rmdir <directory>: Remove an existing directory\n"
       "ls <directory>: List the contents of a directory\n"
       "cat <file>: Display contents of a file\n"
       "touch <file>: Create a new file\n"
       "grep <word> <file>: Search a case-insensitive word within a file\n"
       "calc <expression>: Perform basic arithmetic calculations\n"
       "rand <min> <max>: Generate a random integer between min and max\n");
}

int nate_exit(char **args) { exit(0); }

// int nate_pwd() {
//     char cwd[1024];
//     if (getcwd(cwd, sizeof(cwd)) != NULL) {
//        "%s\n", cwd);
//     } else {
//         perror("getcwd() error");
//     }
// }

int nate_mkdir(char *directory_name) {
  if (mkdir(directory_name, 0777) == -1) {
    perror("nate");
    return -1;
  }
  return 1;
}

int nate_rmdir(char *directory_name) {
  if (rmdir(directory_name) == -1) {
    perror("nate");
    return -1;
  }
  return 1;
}

// This is for ls
int list_directory(const char *dirname) {
  DIR *dir = opendir(dirname);
  if (!dir) {
    fprintf(stderr, "Cannot open %s (%s)\n", dirname, strerror(errno));
    exit(EXIT_FAILURE);
  }
  struct dirent *ent;
  while ((ent = readdir(dir)) != NULL) {
    switch (ent->d_type) {
    case DT_REG:
      printf("%s\n", ent->d_name);
      break;

    case DT_DIR:
      printf("%s/\n", ent->d_name);
      break;

    case DT_LNK:
      printf("%s@\n", ent->d_name);
      break;

    default:
      printf("%s*\n", ent->d_name);
    }
  }
  return 1;
}
// This method is to start the shell as fork allows us to perform a system call
//
int nate_exec(char **args) {
  char command[100];
  int status;

  while (1) {
    printf("Enter command: ");
    fgets(command, 100, stdin);
    if (command[0] == '\n') {
      continue;
    }
    if (command[strlen(command) - 1] == '\n') {
      command[strlen(command) - 1] = '\0';
    }

    pid_t pid = fork();
    if (pid == -1) {
      printf("Error: Failed to fork process\n");
      exit(1);
    } else if (pid == 0) {
      if (system(command) < 0) {
        printf("%s Error: Failed to execute command\n", ERROR_MSG);
        exit(1);
      }
      exit(0);
    } else {
      waitpid(pid, &status, 0);
      if (WIFEXITED(status) && !WEXITSTATUS(status)) {
        printf("%s", SUCCESS_MSG);
      } else {
        printf("%s", ERROR_MSG);
        exit(1);
      }
    }
  }

  return 0;
}
// pid_t child_pid = fork(); // process ID number of its child.
// int status;
// if (child_pid == 0) {
//   if (execvp(args[0], args) == -1) {
//     perror("Nate");
//   }
//   exit(EXIT_FAILURE); // if failed, we exit
// } else if (child_pid < 0) {
//   // fork failed
//   perror("nate");
// } else {
//   do {
//     // parent process
//     waitpid(child_pid, &status, 0);
//   } while (!WIFEXITED(status) && !WIFSIGNALED(status));
// }
// return -1; // only return if all of the checks are verified

// execCommands
//  We are checking for an error rather than an EOF for
//  effeciency
char *nate_read_line(void) {
  char *buffer = malloc(sizeof(char) * TOKEN_BUFSIZE);
  int position = 0;
  int c;

  if (!buffer) {
    fprintf(stderr, "nate: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    // If we hit EOF, replace it with a null character and return
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }

    // Increment position and check if we have exceeded the buffer size
    position++;

    if (position >= TOKEN_BUFSIZE) {
      fprintf(stderr, "nate: input too long\n");
      exit(EXIT_FAILURE);
    }
  }
}

// parse the input command
char **nate_parse(char *my_line) {
  int bufsize = TOKEN_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token;

  if (!tokens) {
    fprintf(stderr, "nate: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(my_line, " \t\r\n\a");
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += TOKEN_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char *));
      if (!tokens) {
        fprintf(stderr, "nate: allocation error\n");
        exit(EXIT_FAILURE);
      } else {
        // Do something with token
        token = strtok(NULL, " \t\r\n");
        if (token != NULL) {
          printf("%s\n", token);
        }
      }
    }
    token = strtok(NULL, " \t\r\n\a");
  }
  tokens[position] = NULL;
  return tokens;
}

void nate_loop(void) {

  char *my_line;
  char **args;
  int status;
  do {
    printf("<->");
    my_line = nate_read_line();
    args = nate_parse(my_line);
    status = nate_exec(args);
    free(my_line);
    free(args);
  } while (status);
}

int main(int argc, char **argv) {
  greeting();
  nate_loop();
  return 0;
}

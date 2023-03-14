// Initial code for shell along with header files which maybe required for reference
# include <stdio.h> 
# include <stdlib.h>   // used to execute subprocess and commands
# include <string.h>   
# include <unistd.h>   // used for exit, getcwd,read, write, exec
# include <sys/wait.h>  
# include <sys/types.h>//pid_T
# include <dirent.h> // for ls
# include <errno.h>
#include <stdbool.h>// for while loop
# include <sys/stat.h>// for mkdir
# include <fcntl.h>  // used for open
#define TOKEN_BUFSIZE 1024 
//I implement this to assume a certain size for input
// read command line for the input line in shell

// Define the command functions

int nate_cd(char **args);
int nate_exit(char **args);
int nate_help(char **args);
int nate_mkdir(char *directory_name);
int nate_rmdir(char *directory_name);
int list_directory(const char *dirname);
int nate_exec(char **args);
char **nate_parse(char *my_line);
void nate_loop(void);

char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "mkdir",
};

int (*builtin_func[]) (char **) = {
  &nate_cd,
  &nate_exit,
  &nate_help,
  &nate_mkdir
};

int nate_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int nate_cd(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "nate cd: invalid argument\n");
    }else{
        if (chdir(args[1]) != 0)
        {
            perror("nate: cd");
        }
    }
    return 1; // always return 1 in order to continue executing
}
//this is for help 
int nate_help(char **args) {
    // char *helptext = "Nate - OS Shell."
    //         "The following commands are available:\n"
    //       " cd: Change the working directory.\n"
    //       " ls:        List all of the files in current directory.\n"
    //       " pwd:       Using this command will display the current working directory.\n"
    //       " mkdir: This command will be used to create a folder within directory. It should give an error if argument to this command is empty.\n"
    //       " rmdir: This command will be used to remove a folder within directory. It should give an error if there is  an empty argument to this command or if the directory doesnot exists.\n"
    //       " exit:     Exit the shell.\n" 
    //       " help:     Print this help text.\n";
    //printf("%s", helptext);

    for (int i = 0; i < nate_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }
    return 1;
}
int nate_exit(char **args){
    return 0;
}

// int nate_pwd() {
//     char cwd[1024];
//     if (getcwd(cwd, sizeof(cwd)) != NULL) {
//         printf("%s\n", cwd);
//     } else {
//         perror("getcwd() error");
//     }
// }

int nate_mkdir(char *directory_name){
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



//this is for cd 

//This is for ls
int list_directory(const char *dirname)
{
    DIR *dir = opendir(dirname);
    if (!dir)
    {
        fprintf(stderr, "Cannot open %s (%s)\n", dirname, strerror(errno));
        exit(EXIT_FAILURE);
    }
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL)
    {
        switch (ent->d_type)
        {
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
}
//This method is to start the shell as fork allows us to perform a system call
//
int nate_exec(char **args)
{
    pid_t child_pid = fork();// process ID number of its child. 
    int status =0;
    if (child_pid == 0)
    {
        if (execvp(args[0], args) == -1)
            { 
                perror("Nate");
            }
        exit(EXIT_FAILURE); // if failed, we exit
        }else if (child_pid < 0) {
        // fork failed
        perror("nate");
    } else {
        do {
        // parent process
        waitpid(child_pid, NULL, 0);
        }while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;//only return if all of the checks are verified
}


//We are checking for an error rather than an EOF for
//effeciency
char *nate_read_line(void)
{
    char *my_line = NULL;
    size_t bufsize = 0;
    ssize_t reader = getline(&my_line, &bufsize, stdin);

    if (reader == -1) {// LOOK HERE
        if (ferror(stdin))
        {
            fprintf(stderr, "Error reading input\n");
        }else{
            perror("readline");
            exit(EXIT_FAILURE);
        }
    }
    return my_line;
}

// parse the input command
char **nate_parse(char *my_line) {
    int bufsize = TOKEN_BUFSIZE; // initial size for tokens array
    int pos = 0;
    char **tokens = malloc(bufsize * sizeof(char*));//allocate for tokens array
    char *token;

    if (!tokens)
    {
        fprintf(stderr, "nate: allocation error\n");
       // free(my_line);//in case of a failed allocation 
        exit(EXIT_FAILURE);
    }
        token = strtok_r(my_line, " \t\r\n", &my_line);//strtok_r (Secure Coding) for thread safety
        while (token != NULL) {
            tokens[pos] = token;
            pos++;
            if (pos >= bufsize) {
                bufsize += TOKEN_BUFSIZE; // increase buffer size by 16
                tokens = realloc(tokens, bufsize * sizeof(char *));//increase the buffer size when array becomes full 
                if (!tokens) {
                    fprintf(stderr, "nate: failed to allocate memory\n");
                    exit(EXIT_FAILURE);
                }
            }else{
                token = strtok(NULL, " \t\r\n");
            }
        }
    tokens[pos] = NULL; // add NULL at the end of tokens array
    return tokens;
}

void nate_loop(void){

    printf("Welcome to Assignment 1 ! \n");
    puts("**************************************\n");
    printf("%s","Welcome to Nate's Shell! ");
    puts("**************************************\n");
    //define variables to ensure they exist outside of loop
    char *my_line;
    char **args;
    int status = 0;
    //start the loop 
    do {
        printf("<-> ");//to make it look cool
        my_line = nate_read_line();//reads in a line
        args = nate_parse(my_line);
        nate_exec(args);
        //free up any space
        free(my_line); //frees the memory allocated for the input 
        free(args);
    }while(true);
}

int main(int argc, char** argv)
{
    
    nate_loop();
    return EXIT_SUCCESS; 
}

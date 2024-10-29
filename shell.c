#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h> 
#define MAX_LINE 1024     // Maximum length of command line input
#define MAX_ARGS 128      // Maximum number of arguments
#define MAX_CMDS 16       // Maximum number of piped commands
#define MAX_SEQ_CMDS 16   // Maximum number of sequential commands


pid_t background_pids[MAX_CMDS];
int background_count = 0;


void parse_input(char *input, int *background, char **input_file, char **output_file, char ****commands, int *num_cmds);
void execute_command(char **args, int background, char *input_file, char *output_file);
void execute_pipe(char ***commands, int num_cmds, int background, char *input_file, char *output_file);
void parse_command_sequence(char *input, char ****seq_commands, char *seq_operators, int *num_seq_cmds);
void execute_command_sequence(char ***seq_commands, char *seq_operators, int num_seq_cmds);
int redirect_input(char *input_file);
int redirect_output(char *output_file);
void handle_sigchld(int sig);
void execute_wait();
void execute_bgpids();
void execute_pid();
void execute_ppid();
void execute_pidof(char *command);

// Main function 
int main() {
    setenv("PATH", "/usr/local/bin:/usr/bin:/bin", 1);

    // Setup SIGCHLD handler to reap background processes
    struct sigaction sa;
    sa.sa_handler = &handle_sigchld;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    char input[MAX_LINE];
    char ***seq_commands = NULL;          // pointers to command sequences
    char seq_operators[MAX_SEQ_CMDS];     // store the operators between commands (&&, ;)
    int num_seq_cmds;                      // Number of command sequences

    while (1) {
        printf("Shiyadav@iu> "); 
        fflush(stdout);

        if (fgets(input, MAX_LINE, stdin) == NULL) {
            break;  // Exit if no input is received
        }

        input[strcspn(input, "\n")] = 0;  

        if (input[0] == '\0') {
            continue;  
        }

        if (strcmp(input, "exit") == 0) {
            break;  
        }

        if (strcmp(input, "wait") == 0) {
            execute_wait();  // wait command
            continue;
        }

        if (strcmp(input, "bgpids") == 0) {
            execute_bgpids();  // background processes
            continue;
        }

        if (strcmp(input, "pid") == 0 || strcmp(input, "$$") == 0) {
            execute_pid();  // shell's PID
            continue;
        }

        if (strcmp(input, "ppid") == 0) {
            execute_ppid();  //shell's parent PID
            continue;
        }

        // Check if input is for pidof command
        if (strncmp(input, "pidof ", 6) == 0) {
            execute_pidof(input + 6); 
            continue;
        }

                num_seq_cmds = 0;
        parse_command_sequence(input, &seq_commands, seq_operators, &num_seq_cmds);

       
        execute_command_sequence(seq_commands, seq_operators, num_seq_cmds);
    }

    printf("Exiting shell...\n");
    return 0;
}

// handle sigchld (background process)
void handle_sigchld(int sig) {
    (void)sig; // Suppress unused parameter warning
    pid_t pid;
    int status;

    //terminated child process
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
            for (int i = 0; i < background_count; i++) {
            if (background_pids[i] == pid) {
                background_pids[i] = 0;
                break;
            }
        }
    }
}


void execute_pid() {
    printf("Current shell PID: %d\n", getpid());
}


void execute_ppid() {
    printf("Parent PID: %d\n", getppid());
}

void execute_bgpids() {
    printf("Background processes:\n");
    for (int i = 0; i < background_count; i++) {
        if (background_pids[i] != 0) {
            printf("PID: %d\n", background_pids[i]);
        }
    }
}


void execute_wait() {
    for (int i = 0; i < background_count; i++) {
        if (background_pids[i] != 0) {
            printf("Waiting for PID: %d\n", background_pids[i]);
            waitpid(background_pids[i], NULL, 0);
            background_pids[i] = 0;  // Mark as completed
        }
    }
    background_count = 0;  
    printf("All background processes have completed.\n");
}


void execute_pidof(char *command) {
    DIR *proc_dir;
    struct dirent *entry;
    char cmdline_path[PATH_MAX];
    char cmdline[256];
    struct stat statbuf;  // To store file status information

    proc_dir = opendir("/proc");
    if (proc_dir == NULL) {
        perror("Failed to open /proc directory");
        return;
    }

    printf("PIDs of '%s':\n", command);
    while ((entry = readdir(proc_dir)) != NULL) {
        
        snprintf(cmdline_path, sizeof(cmdline_path), "/proc/%s", entry->d_name);

        if (stat(cmdline_path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode) && atoi(entry->d_name) > 0) {
            snprintf(cmdline_path, sizeof(cmdline_path), "/proc/%s/cmdline", entry->d_name);
            FILE *cmdline_file = fopen(cmdline_path, "r");
            if (cmdline_file) {
                if (fgets(cmdline, sizeof(cmdline), cmdline_file) != NULL) {
                    if (strstr(cmdline, command) != NULL) {  // Check if command name is in cmdline
                        printf("PID: %s\n", entry->d_name);
                    }
                }
                fclose(cmdline_file);
            }
        }
    }

    closedir(proc_dir);
}


void parse_command_sequence(char *input, char ****seq_commands, char *seq_operators, int *num_seq_cmds) {
    int seq_count = 0;
    char *cmd_start = input;
    char *p = input;

    *seq_commands = malloc(MAX_SEQ_CMDS * sizeof(char**));
    if (*seq_commands == NULL) {
        perror("malloc");
        exit(1);
    }

    while (*p != '\0' && seq_count < MAX_SEQ_CMDS) {
        if (strncmp(p, "&&", 2) == 0) {
           
            *p = '\0'; // Terminate current command
            p += 2;    // Move past '&&'

           
            (*seq_commands)[seq_count] = malloc(2 * sizeof(char*));
            if ((*seq_commands)[seq_count] == NULL) {
                perror("malloc");
                exit(1);
            }
            (*seq_commands)[seq_count][0] = strdup(cmd_start);
            if ((*seq_commands)[seq_count][0] == NULL) {
                perror("strdup");
                exit(1);
            }
            (*seq_commands)[seq_count][1] = NULL;

            
            seq_operators[seq_count++] = '&'; // Using '&' to represent '&&'

            
            cmd_start = p;
        }
        else if (*p == ';') {
            
            *p = '\0'; // Terminate current command
            p += 1;    // Move past ';'

            
            (*seq_commands)[seq_count] = malloc(2 * sizeof(char*));
            if ((*seq_commands)[seq_count] == NULL) {
                perror("malloc");
                exit(1);
            }
            (*seq_commands)[seq_count][0] = strdup(cmd_start);
            if ((*seq_commands)[seq_count][0] == NULL) {
                perror("strdup");
                exit(1);
            }
            (*seq_commands)[seq_count][1] = NULL;

           
            seq_operators[seq_count++] = ';';

            
            cmd_start = p;
        }
        else {
            p++;
        }
    }

    
    if (cmd_start < p && seq_count < MAX_SEQ_CMDS) {
        (*seq_commands)[seq_count] = malloc(2 * sizeof(char*));
        if ((*seq_commands)[seq_count] == NULL) {
            perror("malloc");
            exit(1);
        }
        (*seq_commands)[seq_count][0] = strdup(cmd_start);
        if ((*seq_commands)[seq_count][0] == NULL) {
            perror("strdup");
            exit(1);
        }
        (*seq_commands)[seq_count][1] = NULL;

        
        seq_operators[seq_count] = '\0';
        seq_count++;
    }

    *num_seq_cmds = seq_count;
}


void execute_command_sequence(char ***seq_commands, char *seq_operators, int num_seq_cmds) {
    int prev_status = 0;

    for (int i = 0; i < num_seq_cmds; i++) {
        char ***commands = NULL;
        int background = 0;
        char *input_file = NULL;
        char *output_file = NULL;
        int num_cmds = 0;

       
        parse_input(seq_commands[i][0], &background, &input_file, &output_file, &commands, &num_cmds);

        if (num_cmds > 1) {
            execute_pipe(commands, num_cmds, background, input_file, output_file);
        }
        else if (num_cmds == 1) {
            execute_command(commands[0], background, input_file, output_file);
        }

       
        if (background == 0) {
            int status;
            waitpid(-1, &status, 0);
            prev_status = WIFEXITED(status) ? WEXITSTATUS(status) : 1;
        }
        else {
            prev_status = 0;  // Background process, assume success
        }


        if (i < num_seq_cmds - 1 && seq_operators[i] == '&' && prev_status != 0) {
            break;  // Stop executing if previous command failed and the next command is linked with &&
        }

        
        if (commands != NULL) {
            for (int k = 0; k < num_cmds; k++) {
                if (commands[k] != NULL) {
                    for (int j = 0; commands[k][j] != NULL; j++) {
                        free(commands[k][j]);
                    }
                    free(commands[k]);
                }
            }
            free(commands);
        }
        free(seq_commands[i][0]);  // Free the command sequence
        free(seq_commands[i]);
    }
    free(seq_commands);  // Free the sequence commands array
}

void parse_input(char *input, int *background, char **input_file, char **output_file, char ****commands, int *num_cmds) {
    char *token;
    char *tokens[MAX_LINE];
    int token_count = 0;
    int input_redirect = 0;
    int output_redirect = 0;

    *background = 0;
    *input_file = NULL;
    *output_file = NULL;
    *commands = NULL;
    *num_cmds = 0;

  
    token = strtok(input, " \t");
    while (token != NULL) {
        tokens[token_count++] = strdup(token);
        token = strtok(NULL, " \t");
    }

    
    tokens[token_count] = NULL;

  
    char **cmd_tokens[MAX_CMDS];
    int cmd_count = 0;
    int arg_count = 0;
    char *cmd_args[MAX_ARGS];
    for (int i = 0; i <= token_count; i++) {
        if (tokens[i] == NULL || strcmp(tokens[i], "|") == 0) {
            
            if (arg_count > 0) {
                cmd_args[arg_count] = NULL;
                
                cmd_tokens[cmd_count] = malloc(sizeof(char*) * (arg_count + 1));
                for (int j = 0; j <= arg_count; j++) {
                    cmd_tokens[cmd_count][j] = cmd_args[j];
                }
                cmd_count++;
                arg_count = 0;
            }
            if (tokens[i] != NULL) {
                free(tokens[i]);
                tokens[i] = NULL;
            }
        }
        else if (strcmp(tokens[i], "&") == 0) {
            *background = 1;
            free(tokens[i]);
            tokens[i] = NULL;
        }
        else if (strcmp(tokens[i], "<") == 0) {
            input_redirect = 1;
            output_redirect = 0;
            free(tokens[i]);
            tokens[i] = NULL;
        }
        else if (strcmp(tokens[i], ">") == 0) {
            output_redirect = 1;
            input_redirect = 0;
            free(tokens[i]);
            tokens[i] = NULL;
        }
        else if (input_redirect) {
            *input_file = tokens[i];  // Already duplicated
            tokens[i] = NULL;
            input_redirect = 0;
        }
        else if (output_redirect) {
            *output_file = tokens[i];  // Already duplicated
            tokens[i] = NULL;
            output_redirect = 0;
        }
        else {
            // Regular argument
            cmd_args[arg_count++] = tokens[i];  // Already duplicated
            tokens[i] = NULL;
        }
    }

    
    *commands = malloc(sizeof(char**) * (cmd_count + 1));
    for (int i = 0; i < cmd_count; i++) {
        (*commands)[i] = cmd_tokens[i];
    }
    (*commands)[cmd_count] = NULL; // Null-terminate the commands array
    *num_cmds = cmd_count;

    
    for (int i = 0; i < token_count; i++) {
        if (tokens[i] != NULL) {
            free(tokens[i]);
            tokens[i] = NULL;
        }
    }
}


int redirect_input(char *input_file) {
    int fd = open(input_file, O_RDONLY);
    if (fd < 0) {
        perror("Input file error");
        return -1;
    }
    dup2(fd, STDIN_FILENO);
    close(fd);
    return 0;
}

// output redirection
int redirect_output(char *output_file) {
    int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("Output file error");
        return -1;
    }
    dup2(fd, STDOUT_FILENO);
    close(fd);
    return 0;
}


void execute_command(char **args, int background, char *input_file, char *output_file) {
    pid_t pid;
    int status;

    if (args[0] == NULL) {
        return;
    }

    pid = fork();

    if (pid < 0) {
        perror("Fork error");
        exit(1);
    }

    if (pid == 0) {  // Child process
        // Handle input redirection
        if (input_file != NULL) {
            if (redirect_input(input_file) < 0) {
                exit(1);
            }
        }

        // Handle output redirection
        if (output_file != NULL) {
            if (redirect_output(output_file) < 0) {
                exit(1);
            }
        }


        execvp(args[0], args);
        // If execvp returns, an error occurred
        fprintf(stderr, "Invalid command: %s\n", args[0]);
        exit(1);
    }
    else {  
        if (background == 0) {
            
            waitpid(pid, &status, 0);
        }
        else {
            
            printf("Process running in background with PID: %d\n", pid);
            
            if (background_count < MAX_CMDS) {
                background_pids[background_count++] = pid;
            }
            else {
                fprintf(stderr, "Maximum background processes reached.\n");
            }
        }
    }
}


void execute_pipe(char ***commands, int num_cmds, int background, char *input_file, char *output_file) {
    int pipefd[2 * (num_cmds - 1)];
    pid_t pid[num_cmds];
    int status;

    
    for (int i = 0; i < num_cmds - 1; i++) {
        if (pipe(pipefd + i*2) < 0) {
            perror("Pipe error");
            exit(1);
        }
    }

    for (int i = 0; i < num_cmds; i++) {
        pid[i] = fork();
        if (pid[i] == 0) {
            
            if (i != 0) {
                dup2(pipefd[(i - 1) * 2], STDIN_FILENO);
            }
            else if (input_file != NULL) {
               
                if (redirect_input(input_file) < 0) {
                    exit(1);
                }
            }

            
            if (i != num_cmds - 1) {
                dup2(pipefd[i * 2 + 1], STDOUT_FILENO);
            }
            else if (output_file != NULL) {
                
                if (redirect_output(output_file) < 0) {
                    exit(1);
                }
            }

            
            for (int k = 0; k < 2 * (num_cmds - 1); k++) {
                close(pipefd[k]);
            }

            
            execvp(commands[i][0], commands[i]);
            
            fprintf(stderr, "Invalid command: %s\n", commands[i][0]);
            exit(1);
        }
        else if (pid[i] < 0) {
            perror("Fork error");
            exit(1);
        }
    }

  
    for (int i = 0; i < 2 * (num_cmds - 1); i++) {
        close(pipefd[i]);
    }

    if (background == 0) {
     
        for (int i = 0; i < num_cmds; i++) {
            waitpid(pid[i], &status, 0);
        }
    }
    else {
       
        printf("Process running in background with PIDs: ");
        for (int i = 0; i < num_cmds; i++) {
            printf("%d ", pid[i]);
            // Store all background PIDs
            if (background_count < MAX_CMDS) {
                background_pids[background_count++] = pid[i];
            }
            else {
                fprintf(stderr, "\nMaximum background processes reached.\n");
                break;
            }
        }
        printf("\n");
    }
}

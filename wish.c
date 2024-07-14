/*
sources:
https://brennan.io/2015/01/16/write-a-shell-in-c/
https://www.gnu.org/software/bash/manual/bash.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

// Error message to be used throughout the shell
char error_message[30] = "An error has occurred\n";

// Global path variable
char **search_paths = NULL;
int num_paths = 0;

// Function prototypes
char** split_to_args(char *line, int *argc);
void command_execute(char **args);
void command_cd(char **args);
void command_path(char **args);
int handle_redirection(char **args);
void handle_parallel_commands(char *line);

int main(int argc, char const *argv[]) {
    FILE *input = stdin;

    // Initialize the default search path
    num_paths = 1;
    search_paths = malloc(sizeof(char *));
    search_paths[0] = strdup("/bin");

    // More than 1 argument is an error
    if (argc > 2) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }

    // If an argument is provided, open the file for batch mode
    if (argc == 2) {
        input = fopen(argv[1], "r");
        if (input == NULL) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
    }

    char *line = NULL;
    size_t size = 0;
    ssize_t len;

    while (1) {
        if (input == stdin) {
            printf("wish> ");
            fflush(stdout);
        }
        len = getline(&line, &size, input); // Read from input instead of stdin
        if (len == -1) {
            exit(0);
        }
        // Remove newline character
        if (line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        // Handle parallel commands if '&' is found
        if (strchr(line, '&') != NULL) {
            handle_parallel_commands(line);
        } else {
            int arg_count;
            char **args = split_to_args(line, &arg_count);

            if (args[0] == NULL) {
                free(args);
                continue;
            }

            // Handle built-in commands
            if (strcmp(args[0], "exit") == 0) {
                if (args[1] != NULL) {
                    write(STDERR_FILENO, error_message, strlen(error_message));
                } else {
                    free(args);
                    exit(0);
                }
            } else if (strcmp(args[0], "cd") == 0) {
                command_cd(args);
            } else if (strcmp(args[0], "path") == 0) {
                command_path(args);
            } else {
                int saved_stdout = dup(STDOUT_FILENO);
                int saved_stderr = dup(STDERR_FILENO);

                if (handle_redirection(args) == 0) {
                    command_execute(args);
                }

                // Restore original file descriptors
                dup2(saved_stdout, STDOUT_FILENO);
                dup2(saved_stderr, STDERR_FILENO);
                close(saved_stdout);
                close(saved_stderr);
            }
            free(args);
        }
    }

    free(line);
    if (input != stdin) {
        fclose(input);
    }
    return 0;
}

// Function to split the input line into arguments
char** split_to_args(char *line, int *argc) {
    int bufsize = 64;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, " \t");
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += 64;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, " \t");
    }
    tokens[position] = NULL;
    *argc = position;
    return tokens;
}

// Function to execute a command
void command_execute(char **args) {
    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        char *cmd = args[0];
        char full_path[1024];

        // Search for the command in the search paths
        for (int i = 0; i < num_paths; i++) {
            snprintf(full_path, sizeof(full_path), "%s/%s", search_paths[i], cmd);
            if (access(full_path, X_OK) == 0) {
                // Command found, execute it
                execv(full_path, args);
                // If execv returns, there was an error
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(1);
            }
        }

        // Command not found in any path
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    } else if (pid < 0) {
        // Error forking
        write(STDERR_FILENO, error_message, strlen(error_message));
    } else {
        // Parent process
        wait(NULL);
    }
}

// Function to handle 'cd' command
void command_cd(char **args) {
    if (args[1] == NULL || args[2] != NULL) {
        write(STDERR_FILENO, error_message, strlen(error_message));
    } else {
        if (chdir(args[1]) != 0) { // Corrected condition
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
    }
}

// Function to handle 'path' command
void command_path(char **args) {
    // Free the old search paths
    for (int i = 0; i < num_paths; i++) {
        free(search_paths[i]);
    }
    free(search_paths);

    // Count the number of new paths
    num_paths = 0;
    while (args[num_paths + 1] != NULL) {
        num_paths++;
    }

    // Allocate memory for the new search paths
    search_paths = malloc(num_paths * sizeof(char *));
    for (int i = 0; i < num_paths; i++) {
        search_paths[i] = strdup(args[i + 1]);
    }
}

// Function to handle output redirection
int handle_redirection(char **args) {
    int i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], ">") == 0) {
            if (args[i + 1] == NULL || args[i + 2] != NULL) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                return -1;
            }

            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
            if (fd == -1) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                return -1;
            }

            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);

            args[i] = NULL;
            break;
        }
        i++;
    }
    return 0;
}

// Function to handle parallel commands
void handle_parallel_commands(char *line) {
    char *commands[64]; // Adjust size as needed
    int i = 0;

    char *command = strtok(line, "&");
    while (command != NULL && i < 64 - 1) { // Adjust size as needed
        commands[i++] = command;
        command = strtok(NULL, "&");
    }
    commands[i] = NULL;

    for (int j = 0; j < i; j++) {
        int arg_count;
        char **args = split_to_args(commands[j], &arg_count);
        if (args[0] != NULL) {
            if (fork() == 0) {
                handle_redirection(args);
                command_execute(args);
                exit(0);
            }
        }
        free(args);
    }

    for (int j = 0; j < i; j++) {
        wait(NULL);
    }
}

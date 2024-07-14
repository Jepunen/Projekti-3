# `wish` Shell

## Overview
The `wish` shell is a simple command-line interpreter that supports basic shell functionalities, including executing commands, changing directories, modifying the search path, handling parallel commands, and redirecting output.

## Compilation
To compile the `wish` shell, use the following command:
```sh
gcc -o wish wish.c
```

## Usage
```sh
./wish [batch_file]
```
-   If no arguments are provided, `wish` runs in interactive mode.
-   If a `batch_file` is provided, `wish` executes commands from the file.

## Built-in Commands
1. **exit**
   - Usage: `exit`
   - Exits the shell. If any arguments are provided, an error message is displayed.

2. **cd**
   - Usage: `cd <directory>`
   - Changes the current directory to `<directory>`. If the directory is not specified or too many arguments are provided, an error message is displayed.

3. **path**
   - Usage: `path <dir1> <dir2> ...`
   - Sets the search path for commands. If no arguments are provided, no paths are set, meaning no commands can be executed.

## Features
1. **Command Execution**
   - The shell searches for commands in the directories specified by the `path` command. If a command is found, it is executed.

2. **Output Redirection**
   - Commands can redirect their output to a file using the `>` operator.
   - Usage: `command > output_file`
   - If redirection is improperly specified, an error message is displayed.

3. **Parallel Command Execution**
   - Multiple commands can be run in parallel using the `&` operator.
   - Usage: `command1 & command2 & ...`
   - Each command runs in a separate process.

## Error Handling
-   The shell displays a generic error message `An error has occurred\n` for any invalid command or incorrect usage of built-in commands.

## Functions
1. **split_to_args**
   - Splits a command line into arguments.
   - Prototype: `char** split_to_args(char *line, int *argc);`

2. **command_execute**
   - Executes a command by searching in the specified paths.
   - Prototype: `void command_execute(char **args);`

3. **command_cd**
   - Handles the `cd` command.
   - Prototype: `void command_cd(char **args);`

4. **command_path**
   - Handles the `path` command.
   - Prototype: `void command_path(char **args);`

5. **handle_redirection**
   - Handles output redirection.
   - Prototype: `int handle_redirection(char **args);`

6. **handle_parallel_commands**
   - Handles the execution of parallel commands.
   - Prototype: `void handle_parallel_commands(char *line);`

## Example
```sh
wish> path /bin /usr/bin
wish> cd /home/user
wish> ls > output.txt
wish> cat output.txt & echo "Done" & pwd
```

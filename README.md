# Shell

## Basic Shell: dash

Dash is a basic shell, which is an interactive loop that continuously prompts the user with the message `dash>` followed by a space, reads the user's input, executes the command specified on that line of input, and waits for the command to finish. This is repeated until the user types `exit`. The name of the final executable is `dash`.

The shell can be invoked with either no arguments or a single argument; any other number of arguments is an error. Here are two ways to invoke the shell:

1. Interactive mode: `prompt> ./dash` - at this point, `dash` is running and ready to accept commands.

2. Batch mode: `prompt> ./dash batch.txt` - this mode reads input from a batch file named `batch.txt` and executes the commands from it.

In interactive mode, a prompt is printed (`dash>`). In batch mode, no prompt is printed.

The shell creates a process for each new command, except for built-in commands which are discussed below. The shell is capable of parsing a command and running the program corresponding to the command.

### Paths

The user must specify a path variable to describe the set of directories to search for executables. The set of directories that comprise the path are sometimes called the search path of the shell. The path variable contains the list of all directories to search, in order, when the user types a command.

The shell path should contain one directory initially: `/bin`.

The shell does not implement `ls` or other commands (except built-ins). It finds those executables in one of the directories specified by path and creates a new process to run them. To check if a particular file exists in a directory and is executable, the shell uses the `access()` system call. For example, when the user types `ls`, and path is set to include both `/bin` and `/usr/bin`, the shell tries to access `"/bin/ls"`. If that fails, it tries `"/usr/bin/ls"`. If that fails too, it is an error.

### Built-in Commands

Whenever the shell accepts a command, it checks whether the command is a built-in command or not. If it is, it does not execute it like other programs. Instead, the shell invokes the implementation of the built-in command. In this project, the shell implements `exit`, `cd`, and `path` as built-in commands.

- `exit`: When the user types `exit`, the shell simply calls the `exit()` system call with 0 as a parameter. It is an error to pass any arguments to `exit`.

- `cd`: `cd` always takes one argument (0 or >1 args should be signaled as an error). To change directories, the shell uses the `chdir()` system call with the argument supplied by the user. If `chdir` fails, that is also an error.

- `path`: The `path` command takes 0 or more arguments, with each argument separated by whitespace from the others. If the user sets path to be empty, then the shell should not be able to run any programs (except built-in commands). The `path` command always overwrites the old path with the newly specified path.

### Redirection

To redirect the output of a program to a file, the user can use the `>` character. Formally this is named as redirection of standard output. If a user types `ls -la /tmp > output`, the standard output of the `ls` program should be rerouted to the file `output`. In addition, the standard error output of the file should be rerouted to the file `output`.


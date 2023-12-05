//
// Created by oliver on 01/12/23.
//

//
// INCLUDES
//
#include "enseash.h"

//
// GLOBAL VARIABLES
//
BackgroundProcess background_processes[MAX_BACKGROUND_PROCESSES];
int background_process_count = 0;

/**
 * @brief Main program.
 * Info:
 * - To build code in terminal: gcc enseash.c enseash.h -o enseash
 * - To run code in terminal: ./enseash
 * @param argc number of passed arguments.
 * @param argv passed arguments starting with filename.
 * @return
 */
int main(int argc, char *argv[]) {
    int fdo_dm;
    ssize_t ret;
    char *path_default_messages = "../default_messages.txt";
    char buf[BUFSIZE];

    //
    // WELCOME MESSAGE
    //
    // To do the first question without a file:
    //write(STDOUT_FILENO, MESSAGE_BVN, strlen(MESSAGE_BVN));

    // We open our file containing our welcome message
    if ((fdo_dm = open(path_default_messages, O_RDONLY)) == -1){
        // Error management
        perror("open");
        exit(EXIT_FAILURE);
    }

    // We print our welcome message
    while ((ret = read(fdo_dm, buf, BUFSIZE)) > 0) {
        if (write(STDOUT_FILENO, buf, ret) == -1) {
            // Error management
            perror("write");
            exit(EXIT_FAILURE);
        }
    }
    close(fdo_dm);

    //
    // PROMPT
    //
    print(PROMPT);

    //
    // MAIN LOOP
    //
    while (1) {
        ret = read_command(buf);

        if (ret == -1) { // Error management
            perror("read");
            exit(EXIT_FAILURE);
        }
        if (ret == 0) {
            // No input, continue to the next iteration
            continue;
        }

        buf[ret - 1] = '\0'; // We reset the unused values of the buffer

        //
        // EVAL
        //
        if (eval(buf)) {
            exit(EXIT_SUCCESS);
        }

        // Check for ended background processes
        check_background_processes();
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Prints the String passed using the write function.
 * Has error management.
 * @param string string to print.
 */
void print(char *string) {
    if (write(STDOUT_FILENO, string, strlen(string)) == -1) { // Error management
        perror("write");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Reads a command from the user.
 * @param buf buffer to store the command.
 * @return number of bytes read.
 */
ssize_t read_command(char *buf) {
    // read function explanation: https://stackoverflow.com/questions/12126239/read-not-waiting-for-input
    return read(STDIN_FILENO, buf, BUFSIZE);
}

/**
 * @brief Evaluates the commands and updates the prompt.
 * @param command command to evaluate
 * @return
 */
int eval(char *command) {
    int status;
    char prompt[BUFSIZE];

    struct timespec start;
    struct timespec end;
    double duration;

    char *args[BUFSIZE];
    int arg_count = 0;

    // Our custom exit command
    if (!strcmp(command, "exit")) {
        return EXIT_FAILURE;
    }

    // We start our pid timer
    clock_gettime(CLOCK_MONOTONIC, &start);

    // We create a child pid
    pid_t pid = fork();

    if (pid == -1) { // Error management
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        // Child pid
        if (DEBUG) printf("My PID is %i my parent pid is %i\n", getpid(), getppid());

        // We tokenize the input command
        char *token = strtok(command, " \t\n"); // Tokenize using space, tab, and newline as delimiters

        // We handle redirections
        while (token != NULL) {
            if (strcmp(token, "<") == 0) {
                // Input redirection
                token = strtok(NULL, " \t\n"); // Get the next token (filename for input)
                int fd_in = open(token, O_RDONLY);

                if (fd_in == -1) { // Error management
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                dup2(fd_in, STDIN_FILENO);

                close(fd_in);
            } else if (strcmp(token, ">") == 0) {
                // Output redirection
                token = strtok(NULL, " \t\n"); // Get the next token (filename for output)
                int fd_out = open(token, O_WRONLY | O_CREAT | O_TRUNC, 0666);

                if (fd_out == -1) { // Error management
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                dup2(fd_out, STDOUT_FILENO);

                close(fd_out);
            } else if (strcmp(token, "|") == 0) {
                // Pipe redirection
                int pipefd[2];

                if (pipe(pipefd) == -1) { // Error management
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }

                // Create a new process for the command after the pipe
                pid_t pid2 = fork();

                if (pid2 == -1) { // Error management
                    perror("fork");
                    exit(EXIT_FAILURE);
                } else if (pid2 == 0) {
                    // Wait for all children processes to finish
                    while (wait(NULL) > 0);

                    // Child process after the pipe
                    close(pipefd[1]); // Close the write end of the pipe
                    dup2(pipefd[0], STDIN_FILENO); // Redirect standard input to the read end of the pipe
                    close(pipefd[0]); // Close the read end of the pipe

                    // Tokenize the remaining part of the command
                    while ((token = strtok(NULL, " \t\n")) != NULL) {
                        args[arg_count++] = token;
                    }
                    args[arg_count] = NULL; // Null-terminate the arguments array

                    // Execute the command after the pipe
                    if (execvp(args[0], args) == -1) { // Error management
                        perror("execvp");
                        exit(EXIT_FAILURE);
                    }
                    exit(EXIT_SUCCESS);
                } else {
                    // Wait for all children processes to finish
                    while (wait(NULL) > 0);

                    // Parent process before the pipe
                    close(pipefd[0]); // Close the read end of the pipe
                    dup2(pipefd[1], STDOUT_FILENO); // Redirect standard output to the write end of the pipe
                    close(pipefd[1]); // Close the write end of the pipe

                    // Tokenize the first part of the command
                    while ((token = strtok(NULL, " \t\n")) != NULL) {
                        args[arg_count++] = token;
                    }
                    args[arg_count] = NULL; // Null-terminate the arguments array

                    // Execute the command before the pipe
                    if (execvp(args[0], args) == -1) { // Error management
                        perror("execvp");
                        exit(EXIT_FAILURE);
                    }

                    exit(EXIT_SUCCESS);
                }
            } else {
                // Normal argument
                args[arg_count++] = token;
            }

            token = strtok(NULL, " \t\n");
        }

        args[arg_count] = NULL; // Null-terminate the arguments array

        // Execute the command if no pipe redirection
        if (execvp(args[0], args) == -1) {
            // Error management (in debug mode only)
            if (DEBUG) {
                char error_msg[BUFSIZE];
                sprintf(error_msg, "execvp: %s", command);
                perror(error_msg);
            }
            exit(EXIT_FAILURE);
        }
        // We kill the child process
        exit(EXIT_SUCCESS);
    } else {
        // Parent pid
        if (DEBUG) printf("My PID is %i my child pid is %i\n", getpid(), pid);

        // If the command ends with '&', it's a background process
        if (command[strlen(command) - 1] == '&') {
            command[strlen(command) - 1] = '\0'; // Remove the '&'
            launch_background_process(pid, command);
            return EXIT_SUCCESS; // Return without waiting for background process
        }

        //
        // TIME
        //
        // We get the time when the last child exited
        clock_gettime(CLOCK_MONOTONIC, &end);
        // duration = seconds + nanoseconds
        duration = (end.tv_sec - start.tv_sec) / 1e3 + (end.tv_nsec - start.tv_nsec) / 1e6;

        // Wait for all child processes to finish
        while (waitpid(pid, &status, WUNTRACED | WCONTINUED) > 0) {
            if (WIFEXITED(status)) {
                // We add the exit status to the prompt
                sprintf(prompt, PROMPT_EXIT_TIME, WEXITSTATUS(status), duration);
            } else if (WIFSIGNALED(status)) {
                // We add the signal status to the prompt
                sprintf(prompt, PROMPT_SIGN_TIME, WTERMSIG(status), duration);
            } else if (WIFSTOPPED(status)) {
                // Process has been stopped
                sprintf(prompt, "Process stopped\n");
            } else if (WIFCONTINUED(status)) {
                // Process has been resumed
                sprintf(prompt, "Process resumed\n");
            }

            print(prompt);
        }
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Launches a background process.
 * @param pid PID of the background process.
 * @param command Command associated with the background process.
 */
void launch_background_process(pid_t pid, char *command) {
    if (background_process_count >= MAX_BACKGROUND_PROCESSES) {
        fprintf(stderr, "Maximum number of background processes reached.\n");
        return;
    }

    BackgroundProcess *background_process = &background_processes[background_process_count];
    background_process->pid = pid;
    background_process->number = background_process_count + 1;
    strncpy(background_process->command, command, BUFSIZE - 1);

    printf("[%d] %d\n", background_process->number, pid);

    background_process_count++;
}

/**
 * @brief Updates the information of a background process that has ended.
 * @param pid PID of the ended background process.
 * @param status Status of the ended background process.
 */
void update_background_process(pid_t pid, int status) {
    for (int i = 0; i < background_process_count; ++i) {
        if (background_processes[i].pid == pid) {
            // Background process has ended
            printf("[%d]+ Ended: %s\n", background_processes[i].number, background_processes[i].command);

            // Remove the background process from the list
            for (int j = i; j < background_process_count - 1; ++j) {
                background_processes[j] = background_processes[j + 1];
            }

            background_process_count--;

            return;
        }
    }
}

/**
 * @brief Checks for ended background processes.
 */
void check_background_processes() {
    int status;
    pid_t pid;

    // WNOHANG indicates to waitpid not to block the program
    pid = waitpid(-1, &status, WNOHANG);

    if (pid > 0) {
        // Background process with PID pid has ended
        update_background_process(pid, status);
    } else if (pid == -1) {
        // Error while checking background processes
        perror("waitpid");
    }
}

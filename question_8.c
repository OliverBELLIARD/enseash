//
// Created by oliver on 01/12/23.
//

#include <bits/time.h>
#include <time.h>
#include "enseash.h"

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
    // read function wait explanation:
    // https://stackoverflow.com/questions/12126239/read-not-waiting-for-input
    while ((ret = read(STDIN_FILENO,buf,BUFSIZE)) > 0) {
        if (ret == -1) { // Error management
            perror("read");
            exit(EXIT_FAILURE);
        }

        buf[ret-1] = '\0'; // We reset the unused values of the buffer

        //
        // EVAL
        //
        if (eval(buf)) {
            exit(EXIT_SUCCESS);
        }
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

        // Wait for all child processes to finish
        while (wait(NULL) > 0);

        //
        // TIME
        //
        // We get the time when the last child exited
        clock_gettime(CLOCK_MONOTONIC, &end);
        // duration = seconds + nanoseconds
        duration = (end.tv_sec - start.tv_sec) / 1e3 + (end.tv_nsec - start.tv_nsec) / 1e6;

        //
        // PROMPT
        //
        // We update the prompt after all child processes finish
        if (WIFEXITED(status)) {
            // We add the exit status to the prompt
            sprintf(prompt, PROMPT_EXIT_TIME, WIFEXITED(status), duration);
        } else if (WIFSIGNALED(status)) {
            // We add the signal status to the prompt
            sprintf(prompt, PROMPT_SIGN_TIME, WTERMSIG(status), duration);
        } else {
            // By default, we add only the duration of the last command to the prompt
            sprintf(prompt, PROMPT_EXIT_TIME, WIFEXITED(status), duration);
        }

        print(prompt);
    }

    return EXIT_SUCCESS;
}


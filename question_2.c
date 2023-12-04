//
// Created by oliver on 01/12/23.
//

#include <sys/wait.h>
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
    // WELCOME
    //
    // To do the first question without a file:
    //write(STDOUT_FILENO, MESSAGE_BVN, strlen(MESSAGE_BVN));

    // We open our file containing our welcome message
    if ((fdo_dm = open(path_default_messages, O_RDONLY)) == -1){
        perror("open"); exit(EXIT_FAILURE);
    }

    // We print our welcome message
    while ((ret = read(fdo_dm, buf, BUFSIZE)) > 0) {
        if (write(STDOUT_FILENO, buf, ret) == -1) {
            perror("write"); exit(EXIT_FAILURE);
        }
    }

    close(fdo_dm);

    while ((ret = read(STDIN_FILENO,buf,BUFSIZE)) > 0) {
        if (ret == -1) {
            perror("read"); exit(EXIT_FAILURE);
        }

        buf[ret-1] = 0; // We reset the unused values of the buffer
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
    if (write(STDOUT_FILENO, string, strlen(string)) == -1) {
        perror("write"); exit(EXIT_FAILURE);
    }
}

/**
 * @brief Evaluates the commands.
 * @param string command to evaluate
 * @return
 */
int eval(char *string) {
    pid_t pid = fork();

    if (pid == 0) {
        // Child pid
        if (DEBUG) printf("My PID is %i my parent pid is %i\n", getpid(),	getppid());

        // We evaluate the current user input
        execlp(string, string, NULL);
        exit(EXIT_SUCCESS);
    } else if (pid>0){
        // Parent pid
        if (DEBUG) printf("My PID is %i my child pid is %i\n", getpid(), pid);

        wait(NULL); // We wait for the child to finish its process
        //
        // PROMPT
        //
        print(PROMPT);
    }

    return EXIT_SUCCESS;
}

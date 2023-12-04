//
// Created by oliver on 01/12/23.
//

#include <sys/wait.h>
#include "enseash.h"

/*******************************************************
 * To build code in terminal: gcc enseash.c -o ./enseash
 * To run code in terminal: ./enseash
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
    memset(buf,0,strlen(buf)); // We reset buffer
	
    //
    // PROMPT
    //
    print(PROMPT);
        
    while (1) {
        
    }

    return EXIT_SUCCESS;
}

void print(char *string) {
    if (write(STDOUT_FILENO, string, strlen(string)) == -1) {
        perror("write"); exit(EXIT_FAILURE);
    }
}

//
// Created by oliver on 01/12/23.
//

//
// INCLUDES
//
#ifndef ENSEASH_ENSEASH_H
#define ENSEASH_ENSEASH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <bits/time.h>
#include <time.h>

//
// DEFINES
//
#define BUFSIZE 512 // Size of our buffers storing inputs
#define DEBUG 0     // Enables debug mode

#define MESSAGE_BVN "$ ./enseash\nBienvenue dans le Shell ENSEA.\nPour quitter, tapez 'exit'.\n"
#define PROMPT "enseash % "
#define PROMPT_TIME "enseash [%.2fms] %% "
#define PROMPT_EXIT "enseash [exit:%d] %% "
#define PROMPT_SIGN "enseash [sign:%d] %% "
#define PROMPT_EXIT_TIME "enseash [exit:%d|%.2fms] %% "
#define PROMPT_SIGN_TIME "enseash [sign:%d|%.2fms] %% "

#define MESSAGE_SORTIE "Merci pour avoir utilisé ce shell. Bye bye...\n"

// Maximum number of background processes
#define MAX_BACKGROUND_PROCESSES 10

//
// STRUCTS
//
// Structure to store background processes data
typedef struct {
    pid_t pid;                   // Process ID
    int number;                  // Process number
    char command[BUFSIZE];       // Command associated with the process
} BackgroundProcess;

//
// PROTOTYPES
//
int eval(char *);
void print(char *);
ssize_t read_command(char *);
void launch_background_process(pid_t pid, char *command);
void update_background_process(pid_t pid, int status);
void check_background_processes();

#endif //ENSEASH_ENSEASH_H

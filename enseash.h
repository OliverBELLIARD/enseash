//
// Created by oliver on 01/12/23.
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

#define BUFSIZE 512
#define DEBUG 0

#define MESSAGE_BVN "$ ./enseash\nBienvenue dans le Shell ENSEA.\nPour quitter, tapez 'exit'.\n"
#define PROMPT "enseash % "
#define MESSAGE_SORTIE "Merci pour avoir utilisé ce shell. Bye bye...\n"

int eval(char *);
void print(char *);

#endif ENSEASH_ENSEASH_H

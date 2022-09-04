#ifndef LIB_H
#define LIB_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

#define _EXIT_WITH_ERROR(s) {\
        perror((s));\
        exit(EXIT_FAILURE); }

#define MAX_SLAVE_QTY 5
#define BLOCK_QTY 5

typedef struct {
    unsigned int delivered_files;
    unsigned int file_count;
    unsigned int slave_count;
    pid_t slave_pids[MAX_SLAVE_QTY];
    // 0 Lectura ; 1 Escritura
    unsigned int fds[MAX_SLAVE_QTY][2];
} Manager;

#endif
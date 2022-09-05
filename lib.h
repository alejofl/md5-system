#ifndef LIB_H
#define LIB_H
#define  _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/wait.h>
#include <semaphore.h>

#define SLEEP_TIME_FOR_VIEW 2
#define SHM_NAME    "md5_shm"
#define SEM_NAME    "md5_ready"
#define DEF_SEM_VAL 0

#define _EXIT_WITH_ERROR(s) {\
        perror((s));\
        exit(EXIT_FAILURE); }

#define MAX_SLAVE_QTY 5
#define BLOCK_QTY 5
#define SLAVE_BUFFER_SIZE 256

typedef struct {
    unsigned int delivered_files;
    unsigned int file_count;
    unsigned int slave_count;
    unsigned int received_files;
    pid_t slave_pids[MAX_SLAVE_QTY];
    // 0 Lectura ; 1 Escritura
    unsigned int fds[MAX_SLAVE_QTY][2];
} Manager;


typedef struct {
    int fd;
    void * address;
    int size;
    sem_t * sem;
} Shared_Memory;

char ** check_args(int argc, const char *argv[], unsigned int * file_count);

#endif
#ifndef SHM_H
#define SHM_H

#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>

#define SLEEP_TIME_FOR_VIEW 2
#define SHM_NAME    "md5_shm"
#define SEM_NAME    "md5_ready"
#define DEF_SEM_VAL 0

#define OFLAG_CREATE_SHM (O_CREAT | O_EXCL | O_RDWR)
#define OFLAG_OPEN_SHM (O_RDWR)
#define OFLAG_CREATE_SEM (O_CREAT | O_EXCL | O_RDWR)
#define OFLAG_OPEN_SEM (O_RDWR)
#define MODE_SHM_AND_SEM (S_IRUSR | S_IWUSR)
#define PROT_MMAP (PROT_READ | PROT_WRITE)

typedef struct {
    int fd;
    void * address;
    void * current_address;
    int size;
    sem_t * sem;
} Shared_Memory;

sem_t * open_semaphore(char create);
Shared_Memory open_shared_memory(char create, unsigned int file_count);
void close_shared_memory(char delete, Shared_Memory * shm);

#endif

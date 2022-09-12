// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "include/shm.h"
#include "include/lib.h"

sem_t * open_semaphore(char create) {
    int oflag = create ? OFLAG_CREATE_SEM : OFLAG_OPEN_SEM;
    
    sem_t * semaphore = sem_open(SEM_NAME, oflag, MODE_SHM_AND_SEM, DEF_SEM_VAL);
    if (semaphore == SEM_FAILED) {
        _EXIT_WITH_ERROR(create ? "Semaphore Creation Failed" : "Semaphore Opening Failed");
    }

    return semaphore;
}

Shared_Memory open_shared_memory(char create, unsigned int file_count) {
    Shared_Memory shm;

    int oflag = create ? OFLAG_CREATE_SHM : OFLAG_OPEN_SHM;

    if ((shm.fd = shm_open(SHM_NAME, oflag, MODE_SHM_AND_SEM)) == -1) {
        _EXIT_WITH_ERROR(create ? "Shared Memory Creation Failed" : "Shared Memory Opening Failed");
    }
    shm.size = file_count * SLAVE_BUFFER_SIZE;
    if (create && ftruncate(shm.fd, shm.size) == -1) {
        _EXIT_WITH_ERROR("Truncate Failed.\n");
    }
    if ((shm.address = mmap(NULL, shm.size, PROT_MMAP, MAP_SHARED, shm.fd, 0)) == MAP_FAILED) {
        _EXIT_WITH_ERROR(create ? "Mapping Shared Memory Creation Failed" : "Mapping Shared Memory Opening Failed");
    }
    shm.current_address = shm.address;
    shm.sem = open_semaphore(create);
    return shm;
}

void close_shared_memory(char delete, Shared_Memory * shm) {
    if (munmap(shm->address, shm->size) == -1) {
        _EXIT_WITH_ERROR("Shared Memory Unmapping Failed.");
    }
    if (delete && shm_unlink(SHM_NAME) == -1) {
        _EXIT_WITH_ERROR("Shared Memory Elimination Failed.");
    }
    if (sem_close(shm->sem) == -1) {
        _EXIT_WITH_ERROR("Closing sem failed.");
    }
    if (delete && sem_unlink(SEM_NAME) == -1) {
        _EXIT_WITH_ERROR("Unlink sem failed.");
    }
}

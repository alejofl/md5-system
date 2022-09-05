#include "include/shm.h"
#include "include/lib.h"

sem_t * open_semaphore(char create) {
    int oflag = create ? OFLAG_CREATE_SEM : OFLAG_OPEN_SEM;
    
    sem_t * semaphore = sem_open(SEM_NAME, oflag, MODE_SHM_AND_SEM, DEF_SEM_VAL);
    if (semaphore == SEM_FAILED) {
        _EXIT_WITH_ERROR("Semaphore creation failed.");
    }

    return semaphore;
}

Shared_Memory open_shared_memory(char create, unsigned int file_count) {
    Shared_Memory shm;

    int oflag = create ? OFLAG_CREATE_SHM : OFLAG_OPEN_SHM;

    if ((shm.fd = shm_open(SHM_NAME, oflag, MODE_SHM_AND_SEM)) == -1) {
        _EXIT_WITH_ERROR("Shared Memory Creation Failed");
    }
    shm.size = file_count * SLAVE_BUFFER_SIZE;
    if (ftruncate(shm.fd, shm.size) == -1) {
        _EXIT_WITH_ERROR("Truncate Failed.\n");
    }
    if ((shm.address = mmap(NULL, shm.size, PROT_MMAP, MAP_SHARED, shm.fd, 0)) == MAP_FAILED) {
        _EXIT_WITH_ERROR("Mapping Shared Memory Creation Failed.\n");
    }

    shm.sem = open_semaphore(create);
    return shm;
}

//TODO Verificar si hay que desmappear tambien
void close_shared_memory(sem_t * sem) {
    // Eliminamos el shared memory que creamos
    if (shm_unlink(SHM_NAME) == -1) {
        _EXIT_WITH_ERROR("Shared Memory Elimination Failed.");
    }
    if (sem_close(sem) == -1) {
        _EXIT_WITH_ERROR("Closing sem failed.");
    }
    if (sem_unlink(SEM_NAME) == -1) {
        _EXIT_WITH_ERROR("Unlink sem failed.");
    }
}

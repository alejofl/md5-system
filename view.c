// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "include/lib.h"
#include "include/shm.h"

int main(int argc, char const *argv[]) {
    // Desactiva el buffer de STDOUT. 
    setvbuf(stdout, NULL, _IONBF, 0);

    unsigned int file_count;
    if (argc == 2) {
        sscanf(argv[1], "%u", &file_count);
    } else if (argc > 2) {
        _EXIT_WITH_ERROR("Too many arguments for view.");
    } else {
        scanf("%u", &file_count);
    }

    Shared_Memory shm = open_shared_memory(0, file_count);

    // El +1 va por el header.
    for (int i = 0; i < file_count + 1; i++) {
        sem_wait(shm.sem);

        char to_print[SLAVE_BUFFER_SIZE];
        int bytes_read = 0;
        while (((char *) shm.current_address)[bytes_read] != '\n') {
            to_print[bytes_read] = ((char *) shm.current_address)[bytes_read];
            bytes_read++;
        }
        to_print[bytes_read++] = '\n';
        to_print[bytes_read] = 0;
        shm.current_address += bytes_read;

        printf("%s", to_print);
    }

    close_shared_memory(0, &shm);
    
    return 0;
}

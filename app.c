#include "include/lib.h"
#include "include/shm.h"

char ** check_args(int argc, const char *argv[], unsigned int * file_count);
void handle_md5_response(char * md5, Shared_Memory * shm, unsigned int results_fd);
unsigned int create_file_for_results();
void write_header(Shared_Memory * shm, unsigned int results_fd);

int main(int argc, const char *argv[]) {
    // Desactiva el buffer de STDOUT. 
    setvbuf(stdout, NULL, _IONBF, 0);

    unsigned int file_count = 0;
    char ** paths = check_args(argc, argv, &file_count);
    paths = realloc(paths, file_count * sizeof(char *));

    // Permite a view saber el size del shared memory

    Shared_Memory shm = open_shared_memory(1, file_count);

    sleep(SLEEP_TIME_FOR_VIEW);

    printf("%d\n", file_count);
    
    Manager manager;
    manager.delivered_files = 0;
    manager.file_count = file_count;
    manager.slave_count = 0;
    manager.received_files = 0;
    manager.results_fd = create_file_for_results();

    write_header(&shm, manager.results_fd);

    for (int i = 0; manager.delivered_files < manager.file_count && i < MAX_SLAVE_QTY; i++) {
        // Se crean los arrays para los File Descriptors de los pipes.
        int app_to_slave[2];
        int slave_to_app[2];

        if (pipe(app_to_slave) == -1 || pipe(slave_to_app) == -1) {
            _EXIT_WITH_ERROR("Error in pipe(). Try again.")
        }
        pid_t child_pid = fork();
        if (child_pid == 0) { // Es el proceso hijo.
            // Cierro FDs innecesarios.
            close(app_to_slave[1]);
            close(slave_to_app[0]);

            // Para el slave se vuelve transparente el pipe.
            dup2(app_to_slave[0], 0);
            dup2(slave_to_app[1], 1);

            execv("slave", (char **){NULL});
            _EXIT_WITH_ERROR("Error in execv(). Try again.");
        } else if (child_pid > 0) { // Es el proceso padre.
            // Cierro FDs innecesarios
            close(app_to_slave[0]);
            close(slave_to_app[1]);

            manager.slave_count++;
            manager.slave_pids[i] = child_pid;
            manager.fds[i][0] = slave_to_app[0];
            manager.fds[i][1] = app_to_slave[1];

            // Envío paths de archivos.
            write(manager.fds[i][1], paths[manager.delivered_files], strlen(paths[manager.delivered_files]));
            write(manager.fds[i][1], "\n", 1); 
            manager.delivered_files++;
            if (manager.delivered_files != manager.file_count) {
                write(manager.fds[i][1], paths[manager.delivered_files], strlen(paths[manager.delivered_files]));
                write(manager.fds[i][1], "\n", 1); 
                manager.delivered_files++;
            }
        } else {
            _EXIT_WITH_ERROR("Error in fork(). Try again.")
        } 
    }

    // Escuchamos a los slaves
    while (manager.received_files < manager.file_count) {
        fd_set read_fds;
        int nfds = 0;

        // Agregamos todos los fds al fd_set
        for (int i = 0; i < manager.slave_count; i++) {
            FD_SET(manager.fds[i][0], &read_fds); 
            if (nfds < manager.fds[i][0]) {
                nfds = manager.fds[i][0];
            }
        }

        select(nfds + 1, &read_fds, NULL, NULL, NULL);
        char md5[SLAVE_BUFFER_SIZE];

        for (int i = 0; i < manager.slave_count && manager.received_files < manager.file_count; i++) {
            // Con esto sabemos los slaves que terminaron el md5 y podemos mandarles mas
            if (FD_ISSET(manager.fds[i][0], &read_fds) != 0) {
                ssize_t read_ans = read(manager.fds[i][0], md5, SLAVE_BUFFER_SIZE - 1);
                md5[read_ans] = 0;
                for (int i = 0; i < strlen(md5); i++) {
                    if(md5[i] == '\n'){
                        manager.received_files++;
                    }
                }
                
                handle_md5_response(md5, &shm, manager.results_fd);
                
                //IMPORTANT: Si le mandas un write a un fd cerrado hay problemas.
                if (manager.delivered_files != manager.file_count) {
                    write(manager.fds[i][1], paths[manager.delivered_files], strlen(paths[manager.delivered_files])); 
                    write(manager.fds[i][1], "\n", 1);
                    manager.delivered_files++;
                }
            }
            // TODO Printf momentanemo para visualizar datos
            // printf("Received files %d\n", manager.received_files);
            // printf("Total: %d\n", manager.file_count);
        }
    }

    close_shared_memory(0, &shm);
    close(manager.results_fd);
    // Matar procesos esclavos
    for (int i = 0; i < manager.slave_count; i++) {
        kill(manager.slave_pids[i], SIGKILL);
    }

    // Free del malloc de paths
    for (int i = 0; i < manager.file_count; i++) {
        free(paths[i]);
    }
    free(paths);

    return 0;
}

char ** check_args(int argc, const char *argv[], unsigned int * file_count){
    // Chequeamos cantidad de argumentos.
    if (argc < 2) {
        _EXIT_WITH_ERROR("No files to process.");
    }

    // Chequeamos si archivo existe y es un archivo (no directorios).
    struct stat stats;
    char ** paths = malloc(BLOCK_QTY * sizeof(char *)); // TODO: Chequear si tiene que ir acá el malloc.
    
    for (int i = 1; i < argc; i++) {
        int response = stat(argv[i], &stats);
        if (response == 0) {
            if (S_ISREG(stats.st_mode)) {
                if (*file_count % BLOCK_QTY == 0) {
                    paths = realloc(paths, (*file_count + BLOCK_QTY) * sizeof(char *));
                }
                char * str = malloc(strlen(argv[i]));
                paths[(*file_count)++] = strcpy(str, argv[i]);
            }
        } else {
            fprintf(stderr, "Can't open file %s, skipping.", argv[i]);
        }
    }
    return paths;
}

void handle_md5_response(char * md5, Shared_Memory * shm, unsigned int results_fd) {
    int bytes_written;
    if ((bytes_written = sprintf(shm->current_address, "%s", md5)) < 0) {
        _EXIT_WITH_ERROR("Writing in shared memory failed.");
    }
    shm->current_address += bytes_written;
    write(results_fd, md5, bytes_written);

    //FIXME Printf momentaneo para visualizar la salida
    // printf("%s", md5);

    if (sem_post(shm->sem) == -1) {
        close_shared_memory(1, shm);
        _EXIT_WITH_ERROR("Semaphore post failed.");
    }
}   

unsigned int create_file_for_results() {
    unsigned int fd = open(RESULT_FILENAME, RESULT_FLAGS, RESULT_MODE);
    if (fd == -1) {
        _EXIT_WITH_ERROR("File creation failed.");
    }
    return fd;
}

void write_header(Shared_Memory * shm, unsigned int results_fd) {
    char header[SLAVE_BUFFER_SIZE];
    sprintf(header, "%-34s%-34s%-8s\n", "FILENAME", "MD5 HASH", "SLAVE ID");
    handle_md5_response(header, shm, results_fd);
}

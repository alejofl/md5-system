// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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
    char ** tmp_ptr = realloc(paths, file_count * sizeof(char *));
    if (tmp_ptr == NULL) {
        _EXIT_WITH_ERROR("Memory allocation error.");
    } else {
        paths = tmp_ptr;
    }

    Shared_Memory shm = open_shared_memory(1, file_count);

    printf("%u\n", file_count);

    sleep(SLEEP_TIME_FOR_VIEW);
    
    Manager manager;
    manager.delivered_files = 0;
    manager.file_count = file_count;
    manager.slave_count = 0;
    manager.received_files = 0;
    manager.results_fd = create_file_for_results();

    write_header(&shm, manager.results_fd);

    for (int i = 0; manager.delivered_files < manager.file_count && i < MAX_SLAVE_QTY; i++) {
        int app_to_slave[2];
        int slave_to_app[2];

        if (pipe(app_to_slave) == -1 || pipe(slave_to_app) == -1) {
            _EXIT_WITH_ERROR("Error in pipe(). Try again.")
        }
        pid_t child_pid = fork();
        if (child_pid == 0) {
            close(app_to_slave[1]);
            close(slave_to_app[0]);

            dup2(app_to_slave[0], 0);
            dup2(slave_to_app[1], 1);

            execv("slave", (char **){NULL});
            _EXIT_WITH_ERROR("Error in execv(). Try again.");
        } else if (child_pid > 0) {
            close(app_to_slave[0]);
            close(slave_to_app[1]);

            manager.slave_count++;
            manager.slave_pids[i] = child_pid;
            manager.fds[i][0] = slave_to_app[0];
            manager.fds[i][1] = app_to_slave[1];

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

    while (manager.received_files < manager.file_count) {
        fd_set read_fds = {};
        int nfds = 0;

        for (int i = 0; i < manager.slave_count; i++) {
            FD_SET(manager.fds[i][0], &read_fds); 
            if (nfds < manager.fds[i][0]) {
                nfds = manager.fds[i][0];
            }
        }

        select(nfds + 1, &read_fds, NULL, NULL, NULL);
        char read_str[SLAVE_BUFFER_SIZE * 2];
        char md5[SLAVE_BUFFER_SIZE];

        for (int i = 0; i < manager.slave_count && manager.received_files < manager.file_count; i++) {
            if (FD_ISSET(manager.fds[i][0], &read_fds)) {
                ssize_t read_ans = read(manager.fds[i][0], read_str, SLAVE_BUFFER_SIZE - 1);
                if (read_ans == -1) {
                    _EXIT_WITH_ERROR("Error while reading slave output.");
                } else {
                    read_str[read_ans] = 0;
                }

                int md5_idx = 0;
                for (int i = 0; i < read_ans; i++) {
                    md5[md5_idx++] = read_str[i];
                    if (read_str[i] == '\n') {
                        manager.received_files++;
                        md5[md5_idx] = 0;
                        md5_idx = 0;
                        handle_md5_response(md5, &shm, manager.results_fd);
                    }
                }

                if (manager.delivered_files != manager.file_count) {
                    write(manager.fds[i][1], paths[manager.delivered_files], strlen(paths[manager.delivered_files])); 
                    write(manager.fds[i][1], "\n", 1);
                    manager.delivered_files++;
                }
            }
        }
    }

    close_shared_memory(1, &shm);
    close(manager.results_fd);
    for (int i = 0; i < manager.slave_count; i++) {
        kill(manager.slave_pids[i], SIGKILL);
    }

    for (int i = 0; i < manager.file_count; i++) {
        free(paths[i]);
    }
    free(paths);

    return 0;
}

char ** check_args(int argc, const char *argv[], unsigned int * file_count){
    if (argc < 2) {
        _EXIT_WITH_ERROR("No files to process.");
    }

    struct stat stats;
    char ** paths = malloc(BLOCK_QTY * sizeof(char *));
    if (paths == NULL) {
        _EXIT_WITH_ERROR("Memory allocation error.");
    }

    for (int i = 1; i < argc; i++) {
        int response = stat(argv[i], &stats);
        if (response == 0) {
            if (S_ISREG(stats.st_mode)) {
                if (*file_count % BLOCK_QTY == 0) {
                    char ** tmp_ptr = realloc(paths, (*file_count + BLOCK_QTY) * sizeof(char *));
                    if (tmp_ptr == NULL) {
                        _EXIT_WITH_ERROR("Memory allocation error.");
                    } else {
                        paths = tmp_ptr;
                    }
                }
                char * str = malloc(strlen(argv[i]) + 1); // El +1 es para el \0;
                if (str == NULL) {
                    _EXIT_WITH_ERROR("Memory allocation error.");
                }
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

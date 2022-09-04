#include "lib.h"

int main(int argc, const char *argv[]) {
    // Desactiva el buffer de STDOUT. 
    setvbuf(stdout, NULL, _IONBF, 0);

    if (argc < 2) {
        _EXIT_WITH_ERROR("No files to process.")
        return 1;
    }

    // Chequeamos si archivo existe y es un archivo (no directorios)
    unsigned int file_count;
    char ** paths = malloc(BLOCK_QTY * sizeof(char *)); // TODO: Chequear si tiene que ir acá el malloc.
    struct stat stats;
    for (int i = 1; i < argc; i++) {
        int response = stat(argv[i], &stats);
        if (response == 0) {
            if (S_ISREG(stats.st_mode)) {
                if (file_count % BLOCK_QTY == 0) {
                    paths = realloc(paths, (file_count + BLOCK_QTY) * sizeof(char *));
                }
                char * str = malloc(strlen(argv[i]));
                paths[file_count++] = strcpy(str, argv[i]);
            }
        } else {
            fprintf(stderr, "Can't open file %s, skipping.", argv[i]);
        }
    }

    paths = realloc(paths, file_count * sizeof(char *));
    Manager manager;
    manager.delivered_files = 0;
    manager.file_count = file_count;
    manager.slave_count = 0;

    for (int i = 0; manager.delivered_files < manager.file_count && i < MAX_SLAVE_QTY; i++) {
        // Se crean los arrays para los File Descriptors de los pipes.
        int app_to_slave[2];
        int slave_to_app[2];

        if (pipe(app_to_slave) == -1 || pipe(slave_to_app) == -1) {
            _EXIT_WITH_ERROR("Error in pipe(). Try again.")
        } else {
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
                manager.delivered_files++;
                if (manager.delivered_files + 2 != manager.file_count) { // Signfica que el número de archivos es impar (este es el último archivo)
                    write(manager.fds[i][1], paths[manager.delivered_files], strlen(paths[manager.delivered_files])); 
                    manager.delivered_files++;
                }
            } else {
                _EXIT_WITH_ERROR("Error in fork(). Try again.")
            }
        }
    }

    return 0;
}
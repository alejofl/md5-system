#include "include/lib.h"

// TODO: MANEJO DE ERRORES DE POPEN Y PCLOSE
int main(int argc, char const *argv[]) {
    // Desactiva el buffer de STDOUT. 
    setvbuf(stdout, NULL, _IONBF, 0);

    char * file_name = NULL;
    size_t line_cap = 0;
    int file_name_len;

    while ((file_name_len = getline(&file_name, &line_cap, stdin)) > 0) {
        file_name[file_name_len - 1] = 0;
        char command[strlen(MD5_COMMAND) + file_name_len - 1];
        strcpy(command, MD5_COMMAND);
        FILE * md5_ans = popen(strcat(command, file_name), "r");

        char md5[MD5_LENGTH];
        for (int i = 0; i < MD5_LENGTH; i++) {
            md5[i] = getc(md5_ans);
        }
        md5[MD5_LENGTH - 1] = 0;

        pclose(md5_ans);
        printf("%-34s%-34s%-8d\n", file_name, md5, getpid());  
    }
    
    //IMPORTANT: getline() cuando lo llamas con NULL, 0, ... hace el malloc, por eso se debe hacer el free()
    free(file_name);
    return 0;
}

#include "lib.h"


int main(int argc, char const *argv[]) {

    char * fileName = NULL;
    size_t lineCap = 0;

    setvbuf(stdout, NULL, _IONBF, 0);

    while(getline(&fileName, &lineCap, stdin) > 0){
        printf("%d\n", getpid());
    }
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define OPCION_A 0
#define OPCION_B 1

void opcionA(){

    int fd1, fd2, pos;
    char c;
    char buffer[6];
    memset(&buffer, 0, 6);

    fd1 = open("output.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR, 0664);
    write(fd1, "00000", 5);
    for (int i = 1; i < 10; i++) {

        pos = lseek(fd1, 5*i, SEEK_SET);
        if (fork() == 0) {
            /* Child */
            sprintf(buffer, "%d", i*11111);
            lseek(fd1, pos, SEEK_SET);
            write(fd1, buffer, 5);
            close(fd1);
            exit(0);
        } else {
            /* Parent */
            lseek(fd1, 0, SEEK_SET);
        }
    }

	//wait for all childs to finish
    while (wait(NULL) != -1);

    lseek(fd1, 0, SEEK_SET);
    printf("File contents are:\n");
    while (read(fd1, &c, 1) > 0)
        printf("%c", (char) c);
    printf("\n");
    close(fd1);
}

/*
    Proceso padre escribe en las posiciones 10 * i -> Siendo i un indice del 0 al 8
    Proceso hijo i -> el primero en la posicion 5 y el resto en la posicion 10(i - 1) + 5
*/
void opcionB(){

    int fd1, fd2;
    char c;
    char buffer[6];
    memset(&buffer, 0, 6);

    fd1 = open("outputB.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR, 0664);
    write(fd1, "00000", 5);
    for (int i = 1; i < 10; i++) {

        /* Child */
        if (fork() == 0) {
            lseek(fd1, 10*(i - 1) + 5, SEEK_SET);
            sprintf(buffer, "%d", i*11111);
            write(fd1, buffer, 5);
            close(fd1);
            exit(0);
        }
        /* Parent */
        else {
            lseek(fd1, 10*(i - 1), SEEK_SET);
            write(fd1, "00000", 5);
        }
    }

	//wait for all childs to finish
    while (wait(NULL) != -1);

    lseek(fd1, 0, SEEK_SET);
    printf("File contents are:\n");
    while (read(fd1, &c, 1) > 0)
        printf("%c", (char) c);
    printf("\n");
    close(fd1);
}

int main(void){
    
    if(OPCION_A)
        opcionA();
    if(OPCION_B)
        opcionB();

    return 0;
}

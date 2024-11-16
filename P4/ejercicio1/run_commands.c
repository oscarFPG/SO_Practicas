#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define DEBUG 1

typedef enum {manualExecution, fileExecution} tOption;


int getProcessIndexInArray(int* processes, int pid){

    int i = 0;
    while(pid != processes[i])
        ++i;

    return i;
}

pid_t launch_command(char** argv){
    
    pid_t pid = fork();
    if(pid == -1)
        return -1;

    if(pid == 0){
        int status = execvp(argv[0], argv);  // Execute command
        if(status == -1){
            printf("Ocurrió un error al ejecutar el comando %s\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    else
        return pid;
}

char **parse_command(const char *cmd, int* argc) {

    // Allocate space for the argv array (initially with space for 10 args)
    size_t argv_size = 10;
    const char *end;
    size_t arg_len; 
    int arg_count = 0;
    const char *start = cmd;
    char **argv = malloc(argv_size * sizeof(char *));

    if (argv == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    while (*start && isspace(*start)) start++; // Skip leading spaces

    while (*start) {
        // Reallocate more space if needed
        if (arg_count >= argv_size - 1) {  // Reserve space for the NULL at the end
            argv_size *= 2;
            argv = realloc(argv, argv_size * sizeof(char *));
            if (argv == NULL) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
        }

        // Find the start of the next argument
        end = start;
        while (*end && !isspace(*end)) end++;

        // Allocate space and copy the argument
        arg_len = end - start;
        argv[arg_count] = malloc(arg_len + 1);

        if (argv[arg_count] == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strncpy(argv[arg_count], start, arg_len);
        argv[arg_count][arg_len] = '\0';  // Null-terminate the argument
        arg_count++;

        // Move to the next argument, skipping spaces
        start = end;
        while (*start && isspace(*start)) start++;
    }

    argv[arg_count] = NULL; // Null-terminate the array

    (*argc) = arg_count; // Return argc

    return argv;
}

int main(int argc, char *argv[]) {

    tOption option;
    char* filename = NULL;
    char** cmd_argv = NULL;
    int cmd_argc;
    int binary = 0;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s \"command\" [ -b | -x command | -s file ]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char c;
    while( (c = getopt(argc, argv, "bx:s:")) != -1 ){
        switch(c){
        case 'b':
            if(DEBUG)
                printf("Opcion -b\n");

            binary = 1;
            break;

        case 'x':
            if(DEBUG)
                printf("Opcion -x\n");
            
            option = manualExecution;
            cmd_argv = parse_command(optarg, &cmd_argc);
            break;

        case 's':
            if(DEBUG)
                printf("Opcion -s\n");

            option = fileExecution;
            filename = optarg;
            break;

        default:
            if(DEBUG)
                printf("Opcion no reconocida\n");
            break;
        }
    }


    if(option == manualExecution){

        pid_t childProcess = launch_command(cmd_argv);
        wait(NULL);
    
        if(DEBUG)
            printf("Proceso hijo %d terminado\n", childProcess);

        // Liberar memoria
        for (int i = 0; i < cmd_argc; i++)
            free(cmd_argv[i]);
        free(cmd_argv);
    }
    else if(option == fileExecution && !binary){

        char buffer[512];
        memset(&buffer, 0, 512);
        FILE* fd = fopen(filename, "r");
        if(fd == NULL){
            printf("Error al abrir el fichero\n");
            exit(EXIT_FAILURE);
        }

        while( fgets((char *)&buffer, 512, fd) != NULL){
            
            // Almacenar el comando leido
            cmd_argv = parse_command((char *)&buffer, &cmd_argc);

            // Mostrar el comando con todos sus argumentos
            printf("Ejecutando comando: ");
            for(int i = 0; i < cmd_argc; i++)
                printf("%s ", cmd_argv[i]);
            printf("\n");

            // Ejecutar comando
            pid_t childProcess = launch_command(cmd_argv);
            if(waitpid(childProcess, NULL, 0) == -1)
                if(DEBUG) printf("Error al ejecutar el comando %s\n", cmd_argv[0]);
            else
                if(DEBUG) printf("Comando ejecutado con exito!\n");

            // Liberar memoria
            for (int i = 0; i < cmd_argc; i++)
                free(cmd_argv[i]);
            free(cmd_argv); 

            // Limpiar memoria para almacenar el siguiente
            memset((char *)&buffer, 0, sizeof(buffer));
        }

        return 0;
    }
    else if(option == fileExecution && binary){
        
        int procesos[50];
        char buffer[512];
        memset(&buffer, 0, 512);

        FILE* fd = fopen(filename, "r");
        if(fd == NULL){
            printf("Error al abrir el fichero\n");
            exit(EXIT_FAILURE);
        }

        int i = 0;
        while( fgets((char *)&buffer, 512, fd) != NULL){
            
            // Almacenar el comando leido
            cmd_argv = parse_command((char *)&buffer, &cmd_argc);

            // Mostrar el comando con todos sus argumentos
            printf("Running command #%d: ", i);
            for(int i = 0; i < cmd_argc; i++)
                printf("%s ", cmd_argv[i]);
            printf("\n");

            // Ejecutar comando
            pid_t childProcess = launch_command(cmd_argv);

            // Almacenar pid
            procesos[i] = (int)childProcess;

            // Liberar memoria
            for (int i = 0; i < cmd_argc; i++)
                free(cmd_argv[i]);
            free(cmd_argv); 

            // Limpiar memoria para almacenar el siguiente
            memset((char *)&buffer, 0, sizeof(buffer));

            i++;
        }
        
        // Mostrar informacion de los procesos
        int status;
        for(int counter = 0; counter < i; counter++){
            pid_t ID = waitpid(-1, &status, 0);
            int index = getProcessIndexInArray(&procesos, ID);
            printf("Command #%d terminated (pid: %d, status: %d)\n", index, ID, status);
        }
    }

    return EXIT_SUCCESS;
}
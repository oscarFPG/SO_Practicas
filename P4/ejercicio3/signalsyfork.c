#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define NUM_SECONDS 5

/* Programa que temporiza la ejecución de un proceso hijo */
void configureSignal(int signal, siginfo_t* info, void* context){

	if(signal == SIGALRM)
		kill(0, SIGKILL);
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

int main(int argc, char **argv){

	if(argc != 2){
		printf("Error: Introduce un comando para ejecutar\n");
		exit(EXIT_FAILURE);
	}

	int commandSize;
	char** command = parse_command(argv[1], &commandSize);

	// Configure signals
	struct sigaction st;
	st.sa_handler = configureSignal;
	st.sa_flags = 0;
	if( sigaction(SIGALRM, &st, NULL) == -1 ){
		printf("Error al configurar el envio de signals\n");
		exit(EXIT_FAILURE);
	}


	pid_t pid = fork();
	if(pid == 0){
		int status = execvp(command[0], command);
		if(status == -1){
			printf("Error al ejecutar el comando %s\n", command[0]);
			exit(EXIT_FAILURE);
		}
	}
	else{

		// Start the alarm
		alarm(NUM_SECONDS);

		// Wait for the child process
		int status;
		wait(&status);
		printf("Child process terminated with status %d\n", status);

		// Free memory
		for(int i = 0; 0 < commandSize; i++)
			free(command[i]);
		free(command);
	}

	return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>

#define DEBUG 1


typedef enum { first, last } tMode;

typedef struct{
	char* file;
	int N;
	tMode mode;
}tParameters;


void errorMessage(char* msg){
	printf("%s\n", msg);
	exit(EXIT_FAILURE);
}

void mostrarArchivo(char* filename, const tMode mode, const int bytes){

	int fd;
	if( (fd = open(filename, O_RDONLY)) == -1 )
		errorMessage("No se pudo abrir el archivo");

	// Move file pointer
	int status;
	if(mode == first){
		status = lseek(fd, bytes, SEEK_SET);
		if(status == -1)
			errorMessage("Error al posicionar el puntero del archivo");
	}
	else{
		status = lseek(fd, 0 - bytes, SEEK_END);
		if(status == -1)
			errorMessage("Error al posicionar el puntero del archivo");
	}

	// Print file
	char c;
	status = read(fd, &c, sizeof(char));
	if(status == -1)
		errorMessage("Error al leer del fichero");

	while( status != 0 ){
		
		// Mostramos el caracter por pantalla
		printf("%c", c);

		// Leemos otro caracter
		status = read(fd, &c, sizeof(char));
		if(status == -1)
			errorMessage("Error al leer del fichero");
	}
	printf("\n");
	close(fd);
}

int main(int argc, char *argv[]){

	tParameters parameters;
	parameters.N = 0;
	parameters.mode = first;
	parameters.file = NULL;

	int option;
	while( (option = getopt(argc, argv, "f:n:e")) != -1 ){
		switch(option){
		case 'f':
			parameters.file = optarg;
			break;

		case 'n':
			parameters.N = atoi(optarg);
			break;
		
		case 'e':
			parameters.mode = last;
			break;

		default:
			errorMessage("Opcion no encontrada");
		}
	}

	if(DEBUG){
		printf("Archivo a mostrar %s\n", parameters.file);
		printf("Bytes a saltar %d\n", parameters.N);
		if(parameters.N == first)
			printf("Leemos los primeros %d bytes\n", parameters.N);
		else
			printf("Leemos los %d ultimos bytes\n", parameters.N);
			printf("\n");
	}

	mostrarArchivo(parameters.file, parameters.mode, parameters.N);


	return 0;
}

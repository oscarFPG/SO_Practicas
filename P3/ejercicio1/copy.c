#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define DEBUG 1
#define BUFFER_SIZE 512

void errorMessage(char* msg){
	printf("Error: %s\n", msg);
	exit(EXIT_FAILURE);
}

void copy(int fdo, int fdd){

	if(fdo == -1 || fdd == -1)
		errorMessage("Alguno de los ficheros no esta abierto");

	if(DEBUG)
		printf("Making copy...\n");


	char buffer[BUFFER_SIZE];
	memset(&buffer, 0, BUFFER_SIZE);
	int bytes_read = 0;
	int bytes_written = 0;
	while( (bytes_read = read(fdo, &buffer, BUFFER_SIZE)) > 0 ){
		bytes_written = write(fdd, &buffer, bytes_read);
		if(bytes_written == -1)
			errorMessage("Fallo al escribir en el nuevo archivo");
	}

	if(DEBUG)
		printf("Copy finished!\n");
}

int main(int argc, char *argv[]){

	if(argc != 3)
		errorMessage("Arguments must be <source_file> <destination_file>");

	int fdo, fdd;
	char* sourceFile = argv[1];
	char* destinationFile = argv[2];

	if(DEBUG)
		printf("Source %s and destination %s\n", sourceFile, destinationFile);

	// Abrir ficheros
	if((fdo = open(sourceFile, O_RDONLY)) == -1)
		errorMessage("File not found");
	if((fdd = open(destinationFile, O_RDWR | O_CREAT | O_TRUNC, 0644)) == -1)
		errorMessage("Destination file could not be opened");

	// Realizar copia de fdo al fichero fdd
	copy(fdo, fdd);

	// Cerrar ficheros
	close(fdo);
	close(fdd);
	return 0;
}

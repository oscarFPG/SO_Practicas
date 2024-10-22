#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#define DEBUG 1
#define BUFFER_SIZE 512

void errorMessage(char* msg){
	printf("Error: %s\n", msg);
	exit(EXIT_FAILURE);
}

void copy(int fdo, int fdd){

	if(DEBUG)
		printf("Making copy...\n");

	char buffer[BUFFER_SIZE];
	memset(&buffer, 0, BUFFER_SIZE);
	int bytes_read = 0;
	int bytes_written = 0;
	while( (bytes_read = read(fdo, &buffer, BUFFER_SIZE)) > 0 ){
		bytes_written = write(fdd, &buffer, bytes_read);
		if(bytes_written == -1)
			errorMessage("Fallo al escribir en el nuevo archivo, saliendo...");
	}

	if(DEBUG)
		printf("Copy finished!\n");
}

void copy_regular(char *orig, char *dest){

	int fdo = open(orig, O_RDONLY);
	if(fdo == -1)
		errorMessage("El archivo de origen no se pudo abrir");

	int fdd = open(dest, O_RDWR | O_CREAT | O_TRUNC, 0644);
	if(fdd == -1)
		errorMessage("El archivo de destino no se pudo abrir");

	// Hacer copia
	copy(fdo, fdd);

	// Cerrar archivos
	close(fdo);
	close(fdd);
}

void copy_link(char *orig, char *dest){

	if(DEBUG)
		printf("Making symbolic file copy...\n");

	// Create symbolic link that holds the same file pointed by orig
	struct stat info;
	if(lstat(orig, &info) == -1)
		errorMessage("No se pudo obtener informacion del archivo de origen");

	int fileSize = info.st_size + 1;
	char* buffer = malloc(sizeof(char) * fileSize);
	memset(buffer, 0, fileSize);

	// Consultar el enlace simbolico
	if( readlink(orig, buffer, fileSize) == -1 )
		errorMessage("No se pudo obtener informacion del archivo de origen");

	// Crear el enlace simbolico
	if( symlink(buffer, dest) == -1)
		errorMessage("No se pudo crear el enlace simbolico");
	
	// Liberar memoria
	free(buffer);
	if(DEBUG)
		printf("Symbolic file copy finished!\n");
}

int main(int argc, char *argv[]){

	if(argc != 3)
		errorMessage("Argumentos son <source_file> <destination_file>");

	// Variables
	struct stat fileInfo;
	char* sourceFile = argv[1];
	char* destinationFile = argv[2];

	// Check file type
	int status = lstat(sourceFile, &fileInfo);
	if(status == -1)
		errorMessage("No se pudo obtener informacion del archivo de origen");

	// Make a copy based on file type
	switch(fileInfo.st_mode & __S_IFMT){
	case __S_IFLNK:
		copy_link(sourceFile, destinationFile);
		break;

	case __S_IFREG:
		copy_regular(sourceFile, destinationFile);
		break;
	default:
		errorMessage("El archivo de destino debe ser un archivo regular o un enlace simbolico");
		break;
	}

	return 0;
}

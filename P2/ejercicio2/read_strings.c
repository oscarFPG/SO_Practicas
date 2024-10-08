#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor
 *
 * The loadstr() function must allocate memory from the heap to store
 * the contents of the string read from the FILE.
 * Once the string has been properly built in memory, the function returns
 * the starting address of the string (pointer returned by malloc())
 *
 * Returns: !=NULL if success, NULL if error
 */
char* loadstr(FILE* file){

	char c;
	int offset = 0;

	while(fread(&c, 1, 1, file) != 0){
		++offset;
	}

	// Nothing read -> exit
	if(offset == 0)
		return NULL;

	// Restablecer puntero
	fseek(file, 0 - offset, SEEK_CUR);

	// Guardar string leido en el buffer
	char* buffer = malloc(sizeof(char) * offset + 1);
	fread(buffer, sizeof(char), offset, file);
	buffer[offset] = '\0';

	return buffer;
}

int main(int argc, char *argv[]){

	argc--;
	if(argc < 1){
		printf("Error: missing arguments\n");
		exit(EXIT_FAILURE);
	}


	FILE* file = fopen(argv[1], "r");
	if(file == NULL){
		printf("Error: file not found\n");
		exit(EXIT_FAILURE);
	}

	char* buffer = NULL;
	while( (buffer = loadstr(file)) != NULL ){
		printf("%s\n", buffer);
		free(buffer);
	}

	fclose(file);
	return 0;
}
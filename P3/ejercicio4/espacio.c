#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <dirent.h>


/* Forward declaration */
void errorMessage(char* msg);
int isActualOrPrevious(char* dirName);
char* mergeNames(char* path, char* file);
int get_size(char *fname, size_t *blocks);
int get_size_dir(char *fname, size_t *blocks);


/* 
 * Processes all the files in the command line calling get_size on them to
 * obtain the number of 512 B blocks they occupy and prints the total size in
 * kilobytes on the standard output
 */
int main(int argc, char *argv[]){

	struct stat info;
	size_t blocks = 0;
	int status;
	if(argc == 1){	// No recibimos argumentos por la linea de comandos -> recorrer el directorio actual

		status = get_size_dir(".", &blocks);
		if(status == -1)
			errorMessage("Error al recorrer el directorio");

		printf("%dK   .\n", (int)blocks);
	}
	else{	// Mostrar el tamaño de los archivos pasados como parametro

		char* path;
		int i = 1;
		while(i < argc){
			
			status = get_size(argv[i], &blocks);
			if(status == -1)
				errorMessage("Error al recorrer la lista de archivos");

			printf("%dK   %s\n", (int)blocks, argv[i]);
			++i;
		}
	}

	return 0;
}


/**
 * Print an error message and exit the process
 */
void errorMessage(char* msg){
	printf("%s\n", msg);
	exit(EXIT_FAILURE);
}

/**
 * Check if a directory is not "." or ".."
 */
int isActualOrPrevious(char* dirName){
	return (strcmp(dirName, ".") == 0 || strcmp(dirName, "..") == 0) ? 1 : 0;
}

/**
 * Merge the names in the format path\file
 */
char* mergeNames(char* path, char* file){

	int newSize = strlen(path) + strlen(file) + 2;	// Size for both names and the '\' and '\0' terminator
	char* newPath = malloc(sizeof(char) * newSize);
	memset(newPath, 0, newSize);

	int i = 0;
	int firstSize = strlen(path);
	while(i < firstSize){
		newPath[i] = path[i];
		++i;
	}
	newPath[i++] = '/';
	
	int j = 0;
	int secondSize = strlen(file);
	while(j < secondSize){
		newPath[i] = file[j];
		++i;
		++j;
	}

	return newPath;
}

/* Gets in the blocks buffer the size of file fname using lstat. If fname is a
 * directory get_size_dir is called to add the size of its contents.
 */
int get_size(char* fname, size_t* blocks){

	struct stat info;
	if( lstat(fname, &info) == -1 )
		return -1;

	switch (info.st_mode & __S_IFMT){
	case __S_IFDIR:
		return get_size_dir(fname, blocks);
	
	default:
		*blocks = info.st_size;
		return 0;	
	}
}


/* Gets the total number of blocks occupied by all the files in a directory. If
 * a contained file is a directory a recursive call to get_size_dir is
 * performed. Entries . and .. are conveniently ignored.
 */
int get_size_dir(char *dname, size_t *blocks){

	DIR* dir = opendir(dname);
	if(dir == NULL)
		return -1;

	// Primer recorrido de archivo para calcular el tamaño total del directorio
	struct dirent* dirent;
	struct stat info;
	while( (dirent = readdir(dir)) != NULL ){
		
		char* filename = dirent->d_name;
		if(isActualOrPrevious(filename))
			continue;
		
		char* file = mergeNames(dname, filename);
		if(lstat(file, &info) == -1){
			free(file);
			return -1;
		}
		free(file);

		*blocks += info.st_size;
	}
	closedir(dir);

	// Segundo recorrido para mostrar los posibles directorios anidados
	DIR* dir2 = opendir(dname);
	if(dir2 == NULL)
		return -1;

	size_t moreBlocks = 0;
	while( (dirent = readdir(dir2)) != NULL ){
		
		char* filename = dirent->d_name;
		if(isActualOrPrevious(filename))
			continue;

		char* dirName = mergeNames(dname, filename);
		if(lstat(dirName, &info) == -1){
			free(dirName);
			return -1;
		}

		if(S_ISDIR(info.st_mode)){
			if(get_size_dir(dirName, &moreBlocks) == -1){
				free(dirName);
				errorMessage("Se produjo un error al recorrer un directorio");
			}
		}
		free(dirName);
	}
	closedir(dir2);

	return 0;
}
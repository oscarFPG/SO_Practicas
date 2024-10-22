#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <dirent.h>


/* Forward declaration */
void errorMessage(char* msg);
int get_size(char *fname, size_t *blocks);
int get_size_dir(char *fname, size_t *blocks);

/**
 * Print an error message and exit the process
 */
void errorMessage(char* msg){
	printf("%s\n", msg);
	exit(EXIT_FAILURE);
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


}

/* Processes all the files in the command line calling get_size on them to
 * obtain the number of 512 B blocks they occupy and prints the total size in
 * kilobytes on the standard output
 */
int main(int argc, char *argv[]){

	struct stat info;
	size_t blocks;
	int status;
	if(argc == 1){	// No se pasa nada por la linea de comandos -> recorrer el directorio actual

		status = get_size_dir(".", &blocks);
		if(status == -1)
			errorMessage("Error al recorrer el directorio");

		printf("%dK   .\n", blocks);
	}
	else{	// Mostrar el tamaño de los archivos pasados como parametro

		char* path;
		int i = 1;
		while(i < argc){
			
			status = get_size(argv[i], &blocks);
			if(status == -1)
				errorMessage("Error al recorrer la lista de archivos");

			printf("%dK   %s\n", blocks, argv[i]);
			++i;
		}
	}

	return 0;
}
#include <stdio.h>
#include <unistd.h> /* for getopt() */
#include <stdlib.h> /* for EXIT_SUCCESS, EXIT_FAILURE */
#include <string.h>
#include <getopt.h>
#include "defs.h"


/* Assume lines in the text file are no larger than 100 chars */
#define MAXLEN_LINE_FILE 100

int readInteger(FILE* file){

	char c;
	int digits = 0;
	while( fread(&c, 1, 1, file) != 0 && c != '\n' )
		++digits;

	// Restart pointer
	fseek(file, 0 - digits, SEEK_CUR);
	
	// Read number
	char* buffer = malloc(sizeof(char) * digits);
	fread(buffer, sizeof(char), digits, file);

	// Convert to integer
	int number = atoi(buffer);
	printf("%d\n", number);

	return number;
}


int print_text_file(char *path){

	FILE* file = fopen(path, "r");
	if(file == NULL){
		printf("Error: file not found\n");
		exit(EXIT_FAILURE);
	}

	int numStudents = readInteger(file);


	student_t student;
	char buffer[MAXLEN_LINE_FILE];
	memset(&buffer, 0, MAXLEN_LINE_FILE);

	return 0;
}

int print_binary_file(char *path){

	/* To be completed (part B) */
	return 0;
}


int write_binary_file(char *input_file, char *output_file){

	char buffer[MAXLEN_LINE_FILE];
	memset(&buffer, 0, MAXLEN_LINE_FILE);

	FILE* input = fopen(input_file, "r");
	if(input == NULL){
		printf("Error opening the file\n");
		exit(EXIT_FAILURE);
	}

	// Read number of students in the file
	char c;
	fread(&c, sizeof(char), 1, input);
	int numStudents = atoi(&c);
	fread(&c, sizeof(char), 1, input);

	// Read students
	student_t* lista = malloc(sizeof(student_t) * numStudents);
	for(int i = 0; i < numStudents; i++){
		readStudent(input, &lista[i]);
	}
	fclose(input);


	// Opening binary file
	FILE* output = fopen(output_file, "w");
	if(output == NULL){
		printf("Error opening the file\n");
		exit(EXIT_FAILURE);
	}


	// Write number of students
	int status;
	sprintf(&buffer, "%d", numStudents);
	buffer[strlen(buffer)] = '\0';
	status = fwrite(&buffer, sizeof(int), 1, output);
	memset(&buffer, 0, strlen(buffer));
	if(status == -1){
		printf("Error writing in the binary file\n");
		exit(EXIT_FAILURE);
	}

	// Write students data
	for(int i = 0; i < numStudents; i++){

		// Write ID
		sprintf(&buffer, "%d", lista[i].student_id);
		buffer[strlen(buffer)] = '\0';
		status = fwrite(&buffer, sizeof(int), 1, output);
		memset(&buffer, 0, strlen(buffer));
		if(status == -1){
			printf("Error writing in the binary file\n");
			exit(EXIT_FAILURE);
		}

		// Write NIF
		sprintf(&buffer, "%s", lista[i].NIF);
		buffer[strlen(buffer)] = '\0';
		status = fwrite(&buffer, sizeof(char), strlen(buffer), output);
		memset(&buffer, 0, strlen(buffer));
		if(status == -1){
			printf("Error writing in the binary file\n");
			exit(EXIT_FAILURE);
		}		

		// Write name
		sprintf(&buffer, "%s", lista[i].first_name);
		buffer[strlen(buffer)] = '\0';
		status = fwrite(&buffer, sizeof(char), strlen(buffer), output);
		memset(&buffer, 0, strlen(buffer));
		if(status == -1){
			printf("Error writing in the binary file\n");
			exit(EXIT_FAILURE);
		}

		// Write surname
		sprintf(&buffer, "%s", lista[i].last_name);
		buffer[strlen(buffer)] = '\0';
		status = fwrite(&buffer, sizeof(char), strlen(buffer), output);
		memset(&buffer, 0, strlen(buffer));
		if(status == -1){
			printf("Error writing in the binary file\n");
			exit(EXIT_FAILURE);
		}
	}
	char end = '\n';
	fwrite(&end, sizeof(char), 1, output);
	fclose(output);
	return 0;
}

int main(int argc, char *argv[]){

	int ret_code, opt;
	struct options options;

	/* Initialize default values for options */
	options.input_file = NULL;
	options.output_file = NULL;
	options.action = NONE_ACT;
	ret_code = 0;

	/* Parse command-line options (incomplete code!) */
	while ((opt = getopt(argc, argv, "hi:p")) != -1){

		switch (opt){
		case 'h':
			fprintf(stderr, "Usage: %s [ -h ]\n", argv[0]);
			exit(EXIT_SUCCESS);
		case 'i':
			options.input_file = optarg;
			break;
		case 'p':
			options.action = PRINT_TEXT_ACT;
			break;
		default:
			exit(EXIT_FAILURE);
		}
	}

	if (options.input_file == NULL){
		fprintf(stderr, "Must specify one record file as an argument of -i\n");
		exit(EXIT_FAILURE);
	}

	switch (options.action){
	case NONE_ACT:
		fprintf(stderr, "Must indicate one of the following options: -p, -o, -b \n");
		ret_code = EXIT_FAILURE;
		break;
	case PRINT_TEXT_ACT:
		/* Part A */
		ret_code = print_text_file(options.input_file);
		break;
	case WRITE_BINARY_ACT:
		/* Part B */
		ret_code = write_binary_file(options.input_file, options.output_file);
		break;
	case PRINT_BINARY_ACT:
		/* Part C */
		ret_code = print_binary_file(options.input_file);
		break;
	default:
		break;
	}
	exit(ret_code);
}

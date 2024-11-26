#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


#define DEBUG 1
#define FILENAME "ejemplo.txt"
#define CAPACITY 5
#define VIPSTR(vip) ((vip) ? "  vip  " : "not vip")


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;				// Mutex
pthread_cond_t firstStart = PTHREAD_COND_INITIALIZER;			// Condition variable for first start all at the same time
int aforo = 0;													// Counter for people INSIDE the disco

pthread_cond_t vipClientsWait = PTHREAD_COND_INITIALIZER;		// Wait for the vips -> Only if its full
int vipClientsCurrentTurn = 0, vipClientsOnQueue = 0;			// Vip clients turn and max counter

pthread_cond_t normalClientsWait = PTHREAD_COND_INITIALIZER;	// Wait for the normal clients -> If its full or it is some vip waiting on queue
int normalClientsCurrentTurn = 0, normalClientsOnQueue = 0;		// Normal clients turn and max counter

typedef struct{
	int clientType;		// 1 for vip, 0 for the rest
	int turn;			// Turn number of this client
} tCliente;


void errorMessage(char* message){
	printf("%s\n", message);
	exit(EXIT_FAILURE);
}

void enter_normal_client(int id){

	pthread_mutex_lock(&mutex);
		while(vipClientsOnQueue != 0 || aforo != CAPACITY || normalClientsCurrentTurn != id)
			pthread_cond_wait(&normalClientsWait, &mutex);
		
		printf("Normal client with turn %d entering the disco\n", id);
		aforo++;
		normalClientsCurrentTurn++;
	pthread_mutex_unlock(&mutex);
}

void enter_vip_client(int id){

	printf("Vip client with turn %d waiting on the line\n", id);
	pthread_mutex_lock(&mutex);
		
		vipClientsOnQueue++;
		while(id != vipClientsCurrentTurn || aforo == CAPACITY)
			pthread_cond_wait(&vipClientsWait, &mutex);
		
		vipClientsOnQueue--;
		vipClientsCurrentTurn++;
		aforo++;
		
		// Wake up normal clients from here ONLY if there is no more vip clients
		printf("Vip client with turn %d entering the disco\n", id);
		printf("Now the turn is %d and the people inside the disco is %d\n", vipClientsCurrentTurn, aforo);
		pthread_cond_broadcast(&vipClientsWait);

	pthread_mutex_unlock(&mutex);
}

void dance(int id, int isvip){
	printf("Client %2d (%s) dancing in disco\n", id, VIPSTR(isvip));
	sleep((rand() % 3) + 1);
}

void disco_exit(int id, int isvip){

	pthread_mutex_lock(&mutex);
		--aforo;
		pthread_cond_broadcast(&vipClientsWait);
	pthread_mutex_unlock(&mutex);
	printf("Client %s with turn %d exiting the disco!\n", VIPSTR(isvip), id);
}

void* client(void* arg){

	tCliente client = *(tCliente *)arg;

	if(DEBUG)
		printf("Client %s with turn %d created!\n", VIPSTR(client.clientType), client.turn);

	// Stops all clients at the beginning to start at the same time
	pthread_mutex_lock(&mutex);
		pthread_cond_wait(&firstStart, &mutex);
	pthread_mutex_unlock(&mutex);

	// Try to enter to the disco
	if(client.clientType == 1)
		enter_vip_client(client.turn);
	else if(client.clientType == 0)
		enter_normal_client(client.turn);
	else
		errorMessage("Client type not found, terminating program...");

	// Once inside the disco -> dance and leave
	dance(client.turn, client.clientType);
	disco_exit(client.turn, client.clientType);
	pthread_exit(NULL);
}

int main(int argc, char *argv[]){

	tCliente* clientes = NULL;
	pthread_t* hilos = NULL;
	FILE* file = NULL;
	int numberOfClients = 0;

	// Open file
	file = fopen(FILENAME, "r");
	if(file == NULL)
		errorMessage("File not found");

	// Read number of clients in the file
	if(fscanf(file, "%d", &numberOfClients) == -1)
		errorMessage("Number of clients cannot be read");

	// Read and create threads for each client
	clientes = malloc(numberOfClients * sizeof(tCliente));
	hilos = malloc(numberOfClients * sizeof(pthread_t));
	for(int i = 0; i < numberOfClients; i++){
		
		fscanf(file, "%d", &clientes[i].clientType);
		if(clientes[i].clientType == 1)
			clientes[i].turn = vipClientsOnQueue++;
		else if(clientes[i].clientType == 0)
			clientes[i].turn = normalClientsOnQueue++;

		// Create client thread
		pthread_create(&hilos[i], NULL, (void* )client, (void *)&clientes[i]);
	}

	// Wake up all the clients on the line at the same time
	pthread_cond_broadcast(&firstStart);
	if(DEBUG)
		printf("Wake up all threads\n");

	// Wait for all threads
	for(int i = 0; i < numberOfClients; i++)
		pthread_join(hilos[i], NULL);

	// Free memory and resources
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&firstStart);
	pthread_cond_destroy(&vipClientsWait);
	pthread_cond_destroy(&normalClientsWait);
	free(clientes);
	fclose(file);

	return 0;
}
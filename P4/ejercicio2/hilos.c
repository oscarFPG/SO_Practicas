#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define setPriority(d) (d % 2 == 0) ? 'P' : 'N'
#define N 10


typedef struct{
	int number;
	char priority;
}tData;

void *thread_usuario(void *arg){

	tData info = *(tData*)arg;
	printf("PID: %d - Number %d - Priority: %c\n", pthread_self(), info.number, info.priority);
	pthread_exit(NULL);
}

int main(int argc, char* argv[]){

	pthread_t threads[N];
	tData info;
	for(int i = 0; i < N; i++){

		// Set thread info
		info.number = i + 1;
		info.priority = setPriority(info.number);

		// Thread call and
		pthread_create(&threads[i], NULL, (void *)thread_usuario, (void *)&info);
		pthread_join(threads[i], NULL);
	}

	return 0;
}
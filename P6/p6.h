#ifndef DISCO_H
#define DISCO_H
#include <pthread.h>		/*	Hilos	*/
#include <signal.h>         /*  Seniales */
#include <sys/mman.h>       /* smh */
#include <sys/stat.h>       /* For mode constants smh*/
#include <fcntl.h>          /* For O_* constants smh*/
#include <semaphore.h>



#define NUMITER 3	//Limite de veces que puede un salvaje comer, luego termina
#define M 10 //Numero de raciones maxima (numero de veces que pueden comer los salvajes hasta que haya que llamar al cocinero)

typedef struct{
	int servings; 		// Cantidad de raciones que quedan 0-M
	int cook_waiting;	// Numero de cocineros esperando
	int savages_waiting;	// Numero de salvajes esperando
}datos_compartidos;

#endif
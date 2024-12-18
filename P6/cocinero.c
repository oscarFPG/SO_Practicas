#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include"p6.h"


sem_t* sem_cook; /* contador del semáforo del cocinero (0 o 1) se inicializa a 0 */
sem_t* sem_savages; /* contador del semáforo de salvajes (0 a nºsalvajes) se inicializa a 0 */
sem_t* sem_mtx; //para controlar quien se mete en la seccion critica

datos_compartidos *mem_comp;

// Volatile para que el compilador que no optimice el acceso a esta variable,
// para que se actualice bien al llamar a sigHandler (sino no cambia)
volatile int finish = 0; 

void putServingsInPot(int servings){

	printf("\tCooker[%lu] putServingsInPot\n", getpid());
	sem_wait(sem_mtx);	//LOCK

	while(mem_comp->servings > 0 && !finish){

		printf("\tCooker[%lu] esperando\n", getpid());
		mem_comp->cook_waiting++;	// Este cocinero se quedad esperando
		sem_post(sem_mtx);		// libera el sem (unlock)
		sem_wait(sem_cook);	// esperar a que los salvajes llamen al cocinero

		if (finish)
			return;
		sem_wait(sem_mtx); 	// esperar a que me devuelvan el mutex (lock)
	}
	

	mem_comp->servings = M;
	printf("\tCooker(%lu) cooked, servings[%d]\n", getpid(), mem_comp->servings);

	while(mem_comp->savages_waiting > 0){	// Broadcast a los salvajes
		sem_post(sem_savages);
		mem_comp->savages_waiting--;
	}

	sem_post(sem_mtx);	//UNLOCK
}

void cook(){

	while(!finish) {
		putServingsInPot(M);
	}
}

void sigHandler(int signo){
	finish = 1; //ponemos la variable finish a 1
	sem_post(sem_cook); // Desbloquea al cocinero si está esperando, por si hay mas
}

int main(int argc, char *argv[]){

	signal(SIGTERM, sigHandler); // Registrar el manejador para SIGTERM
	signal(SIGINT, sigHandler); // Registrar el manejador para SIGINT

	// MEMORIA COMPARTIDA
	int shm_fd; //zona de memoria compartida (SHared Memory)

	// Creamos o abrimos el segmento de memoria compartida
	if((shm_fd = shm_open("mem_comp",O_CREAT | O_RDWR, 0777)) == -1){
		perror("Error shm_open() al crear/abrir la memoria compartida");
		exit(1);
	}
	
	// Una vez creado, ajustamos su tamaño con ftruncate
	ftruncate(shm_fd, sizeof(datos_compartidos));
	
	// Usamos mmap para asignar (mapear) el segmento a una dirección accesible para el proceso.
	// PROT_READ | PROT_WRITE: Permite leer y escribir.
	// MAP_SHARED: Los cambios se reflejan en otros procesos.
	mem_comp = (datos_compartidos*) mmap(0, sizeof(datos_compartidos), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);


	// SEMAFOROS
	// Inicializado en 0 porque que el cocinero debe esperar que los salvajes lo despierten.
	if ((sem_cook = sem_open("/sem_cook", O_CREAT, 0777, 0)) == SEM_FAILED) {
		perror("Error sem_open(sem_cook)");
		goto liberacion;
	}

	// También 0, porque los salvajes no deben comer inicialmente (solo cuando el cocinero les avise)
	if ((sem_savages = sem_open("sem_savages", O_CREAT, 0777, 0)) == SEM_FAILED) {
		perror("Error sem_open(sem_savages)");
		goto liberacion;
	}

	// Inicializado en 1, ya que es un mutex que permite acceso exclusivo a la sección crítica (debe entrar alguien inicalmente)
	if ((sem_mtx = sem_open("sem_mtx", O_CREAT, 0777, 1)) == SEM_FAILED) {
		perror("Error sem_open(sem_mtx)");
		goto liberacion;
	}

	mem_comp->servings = 0;
	mem_comp->cook_waiting = 0;
	mem_comp->savages_waiting = 0;

	//LET HIM COOK! LET HIM COOK NOW!!! 
	//I SAIDDD LEEEET!!! HIM!!! COOK!!! SUENA PHONK TREMENDO
	printf("\tCooker[%lu] empieza\n", getpid());
	cook();

	// LIBERAR Memoria compartida y semaforos
	liberacion:
		if (mem_comp && mem_comp != MAP_FAILED) {
        	munmap(mem_comp, sizeof(datos_compartidos));
		}
		if (shm_fd != -1) {
			close(shm_fd);
			shm_unlink("/mem_comp");
		}
		if (sem_cook != SEM_FAILED) {
			sem_close(sem_cook);
			sem_unlink("/sem_cook");
		}
		if (sem_savages != SEM_FAILED) {
			sem_close(sem_savages);
			sem_unlink("/sem_savages");
		}
		if (sem_mtx != SEM_FAILED) {
			sem_close(sem_mtx);
			sem_unlink("/sem_mtx");
		}

	return 0;
}
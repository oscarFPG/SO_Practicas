#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include"p6.h"


sem_t* sem_cook; /* contador del semáforo del cocinero (0 o 1) se inicializa a 0 */
sem_t* sem_savages; /* contador del semáforo de salvajes (0 a nºsalvajes) se inicializa a 0 */
sem_t* sem_mtx;

datos_compartidos *mem_comp;
int cont_midas = 0;

void eat(void){
	//unsigned long id = (unsigned long) getpid();
	
	printf("Savage %lu eating, servings[%d]\n", getpid(), mem_comp->servings-1);
	//mem_comp->servings--;
	sleep(rand() % 5);
}

int getServingsFromPot(void){
	sem_wait(sem_mtx);		//LOCK

	while(mem_comp->servings == 0){		// Hay que reponer raciones --> avisar cocinero

		printf("Savage %lu avisa al cocinero, no queda comida\n", getpid());
		if(mem_comp->cook_waiting > 0){ // Si un cocinero esta esperando, 
			sem_post(sem_cook);			// se le avisa para que cocine() y
			mem_comp->cook_waiting--; 	// deja de estar en espera porque va a cocinar
		}

		mem_comp->savages_waiting++;	// El salvaje se queda esperando
		sem_post(sem_mtx);				// libera el semaforo (unlock)
		sem_wait(sem_savages);			// Espera a que algun cocinero lo despierte para comer
		sem_wait(sem_mtx);				// Intenta recuperar el semaforo (lock) para comerr
	}

	//mem_comp->servings--;
	mem_comp->servings--;
	sem_post(sem_mtx);
	eat(); //Accede seccion critica, pero como tiene el sem va bien

	//sem_post(sem_mtx);		//UNLOCK
}

void savages(void){

	for(int i = 0; i < NUMITER; i++){
		getServingsFromPot();
	}
}

int main(int argc, char *argv[]){

	//MEMORIA COMPARTIDA
	int shm_fd; //zona de memoria compartida (SHared Memory)

	// Creamos o abrimos el segmento de memoria compartida
	if((shm_fd = shm_open("mem_comp", O_RDWR, 0777)) == -1){
		perror("Error shm_open() al crear/abrir la memoria compartida\n Recuerda ejecutar cocinero primero\n");
		exit(1);
	}
	
	// Una vez creado, ajustamos su tamaño con ftruncate
	ftruncate(shm_fd, sizeof(datos_compartidos));
	
	// Usamos mmap para asignar (mapear) el segmento a una dirección accesible para el proceso.
	// PROT_READ | PROT_WRITE: Permite leer y escribir.
	// MAP_SHARED: Los cambios se reflejan en otros procesos.
	mem_comp = (datos_compartidos*) mmap(0, sizeof(datos_compartidos), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);


	//SEMAFOROS
	// El 0 en sem_open significa que estás abriendo el semáforo sin crearlo.
	// Es equivalente a usar O_RDWR (acceso de lectura y escritura), y solo funciona
	// si el semáforo ya fue creado previamente por otro proceso.
	 if ((sem_cook = sem_open("sem_cook", 0)) == SEM_FAILED) {
        perror("Error sem_open(sem_cook), recuerda ejecutar cocinero primero");
        goto cleanup;
    }

    if ((sem_savages = sem_open("sem_savages", 0)) == SEM_FAILED) {
        perror("Error sem_open(sem_savages), recuerda ejecutar cocinero primero");
        goto cleanup;
    }

    if ((sem_mtx = sem_open("sem_mtx", 0)) == SEM_FAILED) {
        perror("Error sem_open(sem_mtx), recuerda ejecutar cocinero primero");
        goto cleanup;
    }

    // Lógica principal
    savages(); //cuadno haya comido 3 vecces se sale

	printf("Savage %lu finished\n", getpid());

cleanup:
    // Liberar recursos
    if (mem_comp && mem_comp != MAP_FAILED) {
        munmap(mem_comp, sizeof(datos_compartidos));
    }
    if (shm_fd != -1) {
        close(shm_fd);
    }
    if (sem_cook != SEM_FAILED) {
        sem_close(sem_cook);
    }
    if (sem_savages != SEM_FAILED) {
        sem_close(sem_savages);
    }
    if (sem_mtx != SEM_FAILED) {
        sem_close(sem_mtx);
    }


	return 0;
}

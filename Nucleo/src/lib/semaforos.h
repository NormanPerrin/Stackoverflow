#ifndef LIB_SEMAFOROS_H_
#define LIB_SEMAFOROS_H_

#include "globales.h"

typedef struct {
	char *nombre;
	int valor;
	t_queue *bloqueados;
} t_semaforo;

t_semaforo* semaforo_create(char*nombre, int valor);
void semaforo_signal(t_semaforo* semaforo);
int semaforo_wait(t_semaforo* semaforo);
void semaforo_blockProcess(t_queue* colaBloqueados, pcb* procesoEjecutando);

#endif /* LIB_SEMAFOROS_H_ */

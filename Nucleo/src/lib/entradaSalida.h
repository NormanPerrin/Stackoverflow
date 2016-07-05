#ifndef LIB_ENTRADASALIDA_H_
#define LIB_ENTRADASALIDA_H_

#include "globales.h"

// Diccionarios:
t_dictionary * dictionaryIO;
t_dictionary *dictionarySemaphores;

typedef struct{
	char* nombre;
	pthread_mutex_t mutex_io;
	sem_t sem_io;
	int retardo;
	t_queue *bloqueados;
} dataDispositivo;

typedef struct{
	pthread_t hiloID;
	dataDispositivo dataHilo;
} hiloIO;

void bloquearProcesoPorIO(hiloIO* dispositivoIO, pcb* unPcb);
void realizarEntradaSalida(pcb* procesoEjecutando, pedidoIO* datos);
void* entradaSalidaThread(void* dataHilo);
hiloIO* crearHiloIO(int index);
void lanzarIOThreads();
pcb* esperarPorProceso(dataDispositivo* datos);

#endif /* LIB_ENTRADASALIDA_H_ */

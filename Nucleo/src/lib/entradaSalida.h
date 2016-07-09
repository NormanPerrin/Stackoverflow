#ifndef LIB_ENTRADASALIDA_H_
#define LIB_ENTRADASALIDA_H_

#include "globales.h"
#include "funciones.h"

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

typedef struct{
	pcb* proceso;
	int espera;
} proceso_bloqueadoIO;

void bloquearProcesoPorIO(hiloIO* dispositivoIO, proceso_bloqueadoIO* unPcb);
void realizarEntradaSalida(pcb* procesoEjecutando, pedidoIO* datos);
void* entradaSalidaThread(void* dataHilo);
hiloIO* crearHiloIO(int index);
proceso_bloqueadoIO* esperarPorProceso(dataDispositivo* datos);

#endif /* LIB_ENTRADASALIDA_H_ */

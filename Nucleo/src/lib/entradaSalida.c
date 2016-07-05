#include "entradaSalida.h"

void bloquearProcesoPorIO(hiloIO* dispositivoIO, pcb* unPcb){
	pthread_mutex_lock(&dispositivoIO->dataHilo.mutex_io);
	queue_push(dispositivoIO->dataHilo.bloqueados, unPcb);
	pthread_mutex_unlock(&dispositivoIO->dataHilo.mutex_io);
	sem_post(&dispositivoIO->dataHilo.sem_io);
}

pcb* esperarPorProceso(dataDispositivo* datos){
	sem_wait(&datos->sem_io);
	pthread_mutex_lock(&datos->mutex_io);
	pcb* proceso = queue_pop(datos->bloqueados);
	pthread_mutex_unlock(&datos->mutex_io);

	return proceso;
}

void realizarEntradaSalida(pcb* procesoEjecutando, pedidoIO* datos){
	// completar
}

void* entradaSalidaThread(void* dataHilo){
	// completar
	return NULL ;
}

/*** Planificación a corto plazo (PCP) ****/

hiloIO* crearHiloIO(int index){
  hiloIO *hilo = malloc(sizeof(hiloIO));

  hilo->dataHilo.retardo = atoi(config->retardosIO[index]);
  sem_init(&hilo->dataHilo.sem_io, 0, 0);
  pthread_mutex_init(&hilo->dataHilo.mutex_io, NULL );
  hilo->dataHilo.bloqueados = queue_create();
  hilo->dataHilo.nombre = strdup(config->ioID[index]);

  return hilo;
}

void lanzarIOThreads(){
  int i = 0;
  while (config->ioID[i] != '\0'){

      hiloIO *hilo = crearHiloIO(i);
      pthread_create(&hilo->hiloID, NULL, &entradaSalidaThread, (void*) &hilo->dataHilo);
      dictionary_put(dictionaryIO, config->ioID[i], hilo);
      log_info(logger, "Lanzando hilo de IO %d que pertenece al dispositivo %s", i, hilo->dataHilo.nombre);

      i++;
    }
}

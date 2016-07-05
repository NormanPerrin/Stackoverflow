
void semaforo_blockProcess(t_queue* colaBloqueados, pcb* procesoEjecutando){
  pcb* copia = malloc(sizeof *copia);
  memcpy(copia, &(procesoEjecutando), sizeof *copia);
  queue_push(colaBloqueados, copia);
}

t_semaforo* semaforo_create(char*nombre, int valor){
  t_semaforo *semaforo = malloc(sizeof(t_semaforo));
  semaforo->nombre = strdup(nombre);
  semaforo->valor = valor;
  semaforo->bloqueados = queue_create();
  return semaforo;
}

void semaforo_signal(t_semaforo* semaforo){

	semaforo->valor++;

  if (semaforo->valor <= 0){
      pcb* procesoBloqueado = queue_pop(semaforo->bloqueados);
      if (procesoBloqueado != NULL){

          procesoBloqueado->estado = READY;
          encolarPcbAListos(procesoBloqueado);
          free(procesoBloqueado);
        }
    }
}

int semaforo_wait(t_semaforo* semaforo){

	semaforo->valor--;

  if (semaforo->valor < 0){

      return TRUE;
    }
  return FALSE;
}

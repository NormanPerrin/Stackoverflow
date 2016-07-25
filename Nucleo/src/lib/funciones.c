#include "funciones.h"

void iniciarEscuchaDeInotify(){
	if (watch_descriptor > 0 && fd_inotify > 0) {
		inotify_rm_watch(fd_inotify, watch_descriptor);
	}
	fd_inotify = inotify_init();
		if (fd_inotify > 0) {
			watch_descriptor = inotify_add_watch(fd_inotify, RUTA_CONFIG_NUCLEO, IN_MODIFY);
		}
}

int setearValoresDeConfig(t_config * archivoConfig){

	config = (t_configuracion*)malloc(sizeof(t_configuracion));

		if (config_has_property(archivoConfig, "PUERTO_UMC")) {
			config->puertoUMC = config_get_int_value(archivoConfig, "PUERTO_UMC");
		} else {
			log_info(logger, "El archivo de configuracion no contiene la clave PUERTO_UMC.");
			return FALSE;
		}
		if (config_has_property(archivoConfig, "IP_UMC")) {
			config->ipUMC = strdup(config_get_string_value(archivoConfig, "IP_UMC"));
		} else {
			log_info(logger, "El archivo de configuracion no contiene la clave IP_UMC.");
			return FALSE;
		}
		if (config_has_property(archivoConfig, "PUERTO_PROG")) {
			config->puertoPrograma = config_get_int_value(archivoConfig, "PUERTO_PROG");
		} else {
			log_info(logger, "El archivo de configuracion no contiene la clave PUERTO_PROG.");
			return FALSE;
		}
		if (config_has_property(archivoConfig, "PUERTO_CPU")) {
			config->puertoCPU = config_get_int_value(archivoConfig, "PUERTO_CPU");
		} else {
			log_info(logger, "El archivo de configuracion no contiene la clave PUERTO_CPU.");
			return FALSE;
		}
		if (config_has_property(archivoConfig, "QUANTUM")) {
			config->quantum = config_get_int_value(archivoConfig, "QUANTUM");
		} else {
			log_info(logger, "El archivo de configuracion no contiene la clave QUANTUM.");
			return FALSE;
		}
		if (config_has_property(archivoConfig, "QUANTUM_SLEEP")) {
			config->retardoQuantum = config_get_int_value(archivoConfig, "QUANTUM_SLEEP");
		} else {
			log_info(logger, "El archivo de configuracion no contiene la clave QUANTUM_SLEEP.");
			return FALSE;
		}
		if (config_has_property(archivoConfig, "SEM_IDS")) {
			config->semaforosID = config_get_array_value(archivoConfig, "SEM_IDS");
		} else {
			log_info(logger, "El archivo de configuracion no contiene la clave SEM_IDS.");
			return FALSE;
		}
		if (config_has_property(archivoConfig, "SEM_INIT")) {
			config->semaforosValInicial = config_get_array_value(archivoConfig, "SEM_INIT");
		} else {
			log_info(logger, "El archivo de configuracion no contiene la clave SEM_INIT.");
			return FALSE;
		}
		if (config_has_property(archivoConfig, "IO_IDS")) {
			config->ioID = config_get_array_value(archivoConfig, "IO_IDS");
		} else {
			log_info(logger, "El archivo de configuracion no contiene la clave IO_IDS.");
			return FALSE;
		}
		if (config_has_property(archivoConfig, "IO_SLEEP")) {
			config->retardosIO = config_get_array_value(archivoConfig, "IO_SLEEP");
		} else {
			log_info(logger, "El archivo de configuracion no contiene la clave IO_SLEEP.");
			return FALSE;
		}
		if (config_has_property(archivoConfig, "SHARED_VARS")) {
			config->variablesCompartidas = config_get_array_value(archivoConfig, "SHARED_VARS");
		} else {
			log_info(logger, "El archivo de configuracion no contiene la clave SHARED_VARS.");
			return FALSE;
		}
		if (config_has_property(archivoConfig, "STACK_SIZE")) {
			config->cantidadPaginasStack = config_get_int_value(archivoConfig, "STACK_SIZE");
		} else {
			log_info(logger, "El archivo de configuracion no contiene la clave STACK_SIZE.");
			return FALSE;
		}
		config_destroy(archivoConfig); // Libero la estructura archivoConfig

	return TRUE;
}

t_semaforo* semaforo_create(char*nombre, int valor){
  t_semaforo *semaforo = malloc(sizeof(t_semaforo));
  semaforo->nombre = strdup(nombre);
  semaforo->valor = valor;
  semaforo->bloqueados = queue_create();

  return semaforo;
}

void registrarSemaforo(char* name, int value){
	t_semaforo* sem = semaforo_create(name, value);
		dictionary_put(diccionarioSemaforos, sem->nombre, sem);
}

var_compartida* crearVariableCompartida(char* nombre, int valorInicial){
	var_compartida* var = malloc(strlen(nombre)+ 5);
		var->nombre = strdup(nombre);
		var->valor = valorInicial;

	return var;
}

void registrarVariableCompartida(char* name, int value){
	var_compartida* var = crearVariableCompartida(name, value);
	dictionary_put(diccionarioVarCompartidas,var->nombre, var);
}

hiloIO* crearHiloIO(int index){
  hiloIO *hilo = malloc(sizeof(hiloIO));
  	  hilo->dataHilo.retardo = atoi(config->retardosIO[index]);
  	  sem_init(&hilo->dataHilo.sem_io, 0, 0);
  	  	  pthread_mutex_init(&hilo->dataHilo.mutex_io, NULL);
  	  hilo->dataHilo.bloqueados = queue_create();
  	  hilo->dataHilo.nombre = strdup(config->ioID[index]);

  return hilo;
}

int validar_cliente(char *id){
	if(!strcmp(id, "C") || !strcmp(id,"P")) {
				printf("Cliente aceptado.\n");
				return TRUE;
			}
	else {
				printf("Cliente rechazado.\n");
				return FALSE;
			}
}

int validar_servidor(char *id){
	if(!strcmp(id, "U")) {
			printf("Servidor aceptado.\n");
			return TRUE;
		}
	else {
			printf("Servidor rechazado.\n");
			return FALSE;
		}
}

proceso_bloqueadoIO* esperarPorProcesoIO(dataDispositivo* datos){
	sem_wait(&datos->sem_io);
	pthread_mutex_lock(&datos->mutex_io);
		proceso_bloqueadoIO* proceso = queue_pop(datos->bloqueados);
	pthread_mutex_unlock(&datos->mutex_io);

	return proceso;
}

void encolarPcbAListos(pcb* proceso){
	printf("Fin de E/S para el proceso #%d. Moviéndolo a cola de Listos.\n", proceso->pid);
	queue_push(colaListos, proceso);
}

void* entradaSalidaThread(void* dataHilo){
	int tiempoDeEspera;
	dataDispositivo* datos = (dataDispositivo*) dataHilo;
	proceso_bloqueadoIO* pcb_block = NULL;
		while (TRUE){
			pcb_block = esperarPorProcesoIO(datos); // tomo el primer proceso en la cola de espera del dispositivo
			tiempoDeEspera = (datos->retardo * pcb_block->espera) * 1000; // duración de la E/S
			usleep(tiempoDeEspera);
			encolarPcbAListos(pcb_block->proceso); // agrego al proceso a la cola de listos

			planificarProceso();
		}
		return NULL;
}

int obtenerSocketMaximoInicial(){
	int i, j, max_fd_inicial = 0;

	if (fdEscuchaCPU > fdEscuchaConsola) {
		max_fd_inicial = fdEscuchaCPU;
		} else  {
			max_fd_inicial = fdEscuchaConsola;
		}
	if (max_fd_inicial < fd_inotify) {
		max_fd_inicial = fd_inotify;
	}

	// Reviso si el fd de alguna Consola supera al max_fd actual:
	for ( j = 0 ; j < list_size(listaConsolas); j++){
		int fd;
		consola * unaConsola = (consola *)list_get(listaConsolas, j);
		fd = unaConsola->fd_consola;
		if(fd > 0)
			FD_SET(fd , &readfds);
		if(fd > max_fd_inicial)
			max_fd_inicial = fd;
	} // fin for consola

	// Reviso si el fd de algún CPU supera al max_fd actual:
	for ( i = 0 ; i < list_size(listaCPU) ; i++){
		int fd;
		cpu * unCPU = (cpu *)list_get(listaCPU, i);
		fd = unCPU->fd_cpu;
		if(fd > 0)
			FD_SET(fd , &readfds);
		if(fd > max_fd_inicial)
			max_fd_inicial = fd;
	} // fin for cpu

	return max_fd_inicial;
}

void planificarProceso(){

	pthread_mutex_lock(&mutex_planificarProceso);

	int i, asignado = 0;
	for (i = 0; (i < list_size(listaCPU) && asignado == 0); i++){
		// Recorro buscando algún CPU libre:
		cpu * unCPU = (cpu *)list_get(listaCPU, i);
		if (unCPU->disponibilidad == LIBRE){
			// Si hay algún proceso listo para ejecutar...
			if (queue_size(colaListos) > 0){
				// Saco el primer proceso de la cola:
				pcb * unPCB = (pcb*) queue_pop(colaListos);
				unPCB->id_cpu = unCPU->id;
				unCPU->pid = unPCB->pid;
				unCPU->disponibilidad = OCUPADO;

		aplicar_protocolo_enviar(unCPU->fd_cpu, PCB, unPCB);

		log_info(logger, "El proceso #%i ingresa a ejecución en CPU id #%i.", unPCB->pid, unCPU->id);

		asignado = 1;
				}
			}
		} // Si no hay CPU libre sigue de largo.
	pthread_mutex_unlock(&mutex_planificarProceso);
}

pcb * buscarProcesoPorPid(int pid){
	int i;
	pcb * unPcb;
	for (i = 0; i < list_size(listaProcesos); i++){
		unPcb = (pcb *)list_get(listaProcesos, i);
		if(unPcb->pid == pid){
			return unPcb; // la pcb del proceso es unPcb
		}
	}
	return NULL; // no se encontró el proceso
}

int asignarPid(){
	int randomPid = rand() % 1000; // número aleatorio entre 0 y 1000
	while (buscarProcesoPorPid(randomPid) != NULL){
		randomPid = rand() % 1000;
	}
	return randomPid;
}

int solicitarSegmentosAUMC(pcb* nuevoPcb, char* programa){

	int tamanio_programa = strlen(programa)+ 1;
	int cantidad_paginas = tamanio_programa / tamanioPagina;
	int offset_restante = tamanio_programa % tamanioPagina;

	nuevoPcb->paginas_codigo = (offset_restante==0)? cantidad_paginas : cantidad_paginas+1;
	nuevoPcb->paginas_stack = config->cantidadPaginasStack;

	// Le solicito a UMC espacio para el heap del programa y verifico su respuesta:
		inicioPrograma* solicitudDeInicio = (inicioPrograma*) malloc(tamanio_programa+12);
		solicitudDeInicio->paginas = nuevoPcb->paginas_stack + nuevoPcb->paginas_codigo;
		solicitudDeInicio->pid = nuevoPcb->pid;
		solicitudDeInicio->contenido = strdup(programa);
		printf("Solicitando segmentos de código y de stack a UMC para el proceso #%d.\n", nuevoPcb->pid);

		aplicar_protocolo_enviar(fd_UMC, INICIAR_PROGRAMA, solicitudDeInicio);
		free(solicitudDeInicio);

		int* respuestaUMC = NULL;
		int head;
		void * entrada = NULL;

		entrada = aplicar_protocolo_recibir(fd_UMC, &head);

		if(entrada == NULL){ // UMC mandó un msj vacío, significa que se desconectó
			return FALSE;
		}
		if(head == RESPUESTA_PEDIDO){
			respuestaUMC = (int*)entrada;
		}
		// Verifico la respuesta de UMC:
		if(*respuestaUMC == PERMITIDO){
			printf("UMC pudo alocar todos los segmentos del proceso #%d.\n", nuevoPcb->pid);

			return TRUE;
		}
		else{
			printf("UMC no pudo alocar los segmentos del proceso #%d. Rechazando ingreso al sistema...\n", nuevoPcb->pid);

			return FALSE;
		}
}

pcb * crearPcb(char* programa){

	pcb * nuevoPcb = malloc(sizeof(pcb));

	nuevoPcb->pid = asignarPid();

	int respuestaUMC = solicitarSegmentosAUMC(nuevoPcb, programa);
	if(respuestaUMC == FALSE){ // el programa es rechazado del sistema
		free(nuevoPcb);
		return NULL;
	}
	else{
		// La UMC pudo alocar los segmentos del proceso, entonces sigo:
		nuevoPcb->id_cpu = -1; // CPU aún no asignado
		nuevoPcb->quantum = config->quantum;
		nuevoPcb->quantum_sleep = config->retardoQuantum;

		// Analizo con el parser el código del programa para obtener su metadata:
		t_metadata_program* infoProg = metadata_desde_literal(programa);

		nuevoPcb->primerPaginaStack = nuevoPcb->paginas_codigo; // el stack comienza luego del código
		nuevoPcb->paginaActualStack = nuevoPcb->primerPaginaStack;
		nuevoPcb->pc = infoProg->instruccion_inicio;
		printf("Program Counter initial value: '%d'. Proceso #%d.\n", nuevoPcb->pc, nuevoPcb->pid); // TODO: Sacar
		nuevoPcb->stackPointer = 0;
		nuevoPcb->cantidad_instrucciones = infoProg->instrucciones_size;

		printf("Cantidad instrucciones: '%d'. Proceso #%d.\n", nuevoPcb->cantidad_instrucciones, nuevoPcb->pid); // TODO: Sacar

		// Inicializo índice de código:
		nuevoPcb->indiceCodigo = infoProg->instrucciones_serializado;

		// Inicializo índice de stack:
		nuevoPcb->indiceStack = list_create();
		nuevoPcb->cantidad_registros_stack = 0;

		// Inicializo índice de etiquetas:
		nuevoPcb->tamanioIndiceEtiquetas = infoProg->etiquetas_size;

		if (infoProg->cantidad_de_etiquetas > 0){
			nuevoPcb->indiceEtiquetas = infoProg->etiquetas;
			printf("Indice etiquetas: '%s'. Proceso #%d.\n", nuevoPcb->indiceEtiquetas, nuevoPcb->pid); // TODO: Sacar
		} else {
			nuevoPcb->indiceEtiquetas = NULL;
		}
		free(infoProg); infoProg = NULL;

		return nuevoPcb;
	}
}

void aceptarConexionEntranteDeConsola(){

	// Las Conosolas mandan un único msj con el script ni bien se conectan:
	int new_fd = aceptarConexionSocket(fdEscuchaConsola);

	int ret_handshake = handshake_servidor(new_fd, "N");

	if(ret_handshake == FALSE){ // Falló el handshake
		log_error(logger, "Conexión inicial fallida con la Consola fd #%d.", new_fd);
		cerrarSocket(new_fd);
		return;
	}
	// Se conectó una nueva Consola:
	consola * nuevaConsola = malloc(sizeof(consola));
	nuevaConsola->id = new_fd - fdEscuchaConsola;
	nuevaConsola->fd_consola = new_fd;
	log_info(logger,"La Consola #%i se ha conectado.", nuevaConsola->id);

	// Recibo el programa de la nueva Consola:
	int head;
	void * entrada = NULL;
	entrada = aplicar_protocolo_recibir(new_fd, &head);
	int* respuesta = malloc(INT);

	if(head == ENVIAR_SCRIPT){
	 // Creo la PCB del programa y pido espacio para los segmentos a UMC:
		pcb * nuevoPcb = crearPcb((char*) entrada);

		if(nuevoPcb == NULL){ //  UMC no pudo alocar los segmentos del programa, entonces lo rachazo:
			*respuesta = RECHAZADO;
			aplicar_protocolo_enviar(new_fd, PROGRAMA_NEW, respuesta);
			cerrarSocket(new_fd);
			free(respuesta); respuesta = NULL;
			return;
		  }
	// Sí se pudieron alocar los segmentos, entonces la Consola y el PCB ingresan al sistema:
		*respuesta = ACEPTADO;
		aplicar_protocolo_enviar(new_fd, PROGRAMA_NEW, respuesta);
		free(respuesta); respuesta = NULL;

		nuevaConsola->pid = nuevoPcb->pid;
		list_add(listaConsolas, nuevaConsola);

	// Pongo al nuevo programa en la cola de Listos:
		list_add(listaProcesos, nuevoPcb);
		queue_push(colaListos, nuevoPcb);

		planificarProceso();

		  } // fin if head scrpit
		  else{
			  printf("Se espera script de la Consola #%d.\n", nuevaConsola->id);
			  *respuesta = ERROR_CONEXION;
			  aplicar_protocolo_enviar(new_fd, PROGRAMA_NEW, respuesta);
			  free(respuesta); respuesta = NULL;
		  }
}

void aceptarConexionEntranteDeCPU(){

	int new_fd = aceptarConexionSocket(fdEscuchaCPU);

	int ret_handshake = handshake_servidor(new_fd, "N");

	if(ret_handshake == FALSE){ // Falló el handshake
		log_error(logger, "Conexión inicial fallida el CPU fd #%d.", new_fd);
		cerrarSocket(new_fd);
		return;
	}
	// Se conectó un nuevo CPU:
	cpu * nuevoCPU = malloc(sizeof(cpu));
	nuevoCPU->id = new_fd - fdEscuchaCPU;
	nuevoCPU->fd_cpu = new_fd;
	nuevoCPU->disponibilidad = LIBRE;

	// Agrego al CPU a la lista de CPUs:
	list_add(listaCPU, nuevoCPU);
	log_info(logger,"La CPU #%i se ha conectado.", nuevoCPU->id);

	// Le envío el tamaño de stack a usar:
	int* stack_size = malloc(INT);
	*stack_size = config->cantidadPaginasStack;
	aplicar_protocolo_enviar(new_fd, TAMANIO_STACK, stack_size);
	free(stack_size);

	// El nuevo CPU está listo para ejecutar procesos:
	planificarProceso();
}

void atenderCambiosEnArchivoConfig(int* socketMaximo){

	int length, i = 0;
	char buffer[EVENT_BUF_LEN];
	length = read(fd_inotify, buffer, EVENT_BUF_LEN);

	log_info(logger, "Se ha modificado el archivo de configuración.");

	if (length <= 0){

		log_error(logger, "Inotify no pudo leer archivo de configuración.");
		return;

	} else {

		struct inotify_event *event = (struct inotify_event *) &buffer[i];
		t_config * new_config = NULL;
		new_config = config_create(RUTA_CONFIG_NUCLEO);

		int aux_quantum, aux_retardo;

		aux_quantum = config_get_int_value(new_config, "QUANTUM");
		aux_retardo = config_get_int_value(new_config, "QUANTUM_SLEEP");

		if(config->quantum != aux_quantum){
			config->quantum = aux_quantum;
			log_info(logger, "Nuevo valor de Quantum: %d.", config->quantum);
		}
		if(config->retardoQuantum != aux_retardo){
			config->retardoQuantum = aux_retardo;
			log_info(logger, "Nuevo valor de Quantum Sleep: %d.", config->retardoQuantum);
		}

		i += EVENT_SIZE + event->len;
		free(new_config); new_config = NULL;

		FD_CLR(fd_inotify, &readfds);
		iniciarEscuchaDeInotify();

		*socketMaximo = (*socketMaximo < fd_inotify)? fd_inotify : *socketMaximo;
		FD_SET(fd_inotify, &readfds);
		return;
	} // fin else-if
}

void salvarProcesoEnCPU(int id_cpu){

	bool esLaPcbEnEjecucion(pcb * pcb){ return pcb->id_cpu == id_cpu; }

	pcb * unPcb = (pcb *) list_find(listaProcesos, (void *) esLaPcbEnEjecucion);
	if(unPcb != NULL){
		unPcb->id_cpu = -1; // le desasigno CPU
		queue_push(colaListos, unPcb); // la mando de nuevo a cola de listos
	}
}

int envioSenialCPU(int id_cpu){
	int i;
	for(i=0; i<list_size(listaCPU_SIGUSR1); i++){
		int * cpu = list_get(listaCPU_SIGUSR1, i);
		if(*cpu == id_cpu){ // mandó señal
			liberarCPU(list_remove(listaCPU_SIGUSR1, i));
			return TRUE;
			}
	} // fin for
	return FALSE; // no mandó señal
}

int seDesconectoConsolaAsociada(int quantum_pid){
	int i;
	for(i=0; i<list_size(listaProcesosAbortivos); i++){
		int * exec_pid = list_get(listaProcesosAbortivos, i);

		if(*exec_pid == quantum_pid){ // se desconectó la consola asociada

			free(list_remove(listaProcesosAbortivos, i));
			printf("Removiendo al proceso #%i porque se desconectó su Consola.\n", quantum_pid);
			// Le informo a UMC que libere la memoria asignada al programa:
			int index = pcbListIndex(quantum_pid); // indexo el pcb en la lista de procesos
			finalizarPrograma(quantum_pid, index);
			// Libero el PCB del proceso y lo saco del sistema:
			liberarPcbNucleo(list_remove(listaProcesos, index));

			return TRUE;
		}
	} // fin for
	return FALSE; // no se deconectó consola
}

int pcbListIndex(int pid){
	int i;
	pcb * unPcb = NULL;
	for (i = 0; i < list_size(listaProcesos); i++){
		unPcb = (pcb *)list_get(listaProcesos, i);
		if(unPcb->pid == pid){
			return i; // el proceso está en la posición 'i'
		}
	}
	return ERROR; // no se encontró el proceso
}

void finalizarPrograma(int pid, int index){

	if (index != ERROR){
		printf("Liberando memoria asignada al proceso #%d.\n", pid);
		// Aviso a UMC que libere la memoria asignada al proceso:
		int* exit_pid = malloc(INT);
		*exit_pid = pid;
		aplicar_protocolo_enviar(fd_UMC, FINALIZAR_PROGRAMA, exit_pid);
		free(exit_pid);
	}
}

void realizarEntradaSalida(pcb* pcbEjecutada, pedidoIO* datos){

	proceso_bloqueadoIO* pcbIO = malloc(sizeof (proceso_bloqueadoIO));
	hiloIO* dispositivoIO = dictionary_get(diccionarioIO, datos->nombreDispositivo);
		pcbIO->espera = datos->tiempo;
		pcbIO->proceso = pcbEjecutada; // debería hacer memcpy ¿?

		// Bloqueo al proceso por IO:
		pthread_mutex_lock(&dispositivoIO->dataHilo.mutex_io);
		queue_push(dispositivoIO->dataHilo.bloqueados, pcbIO);
		pthread_mutex_unlock(&dispositivoIO->dataHilo.mutex_io);
		sem_post(&dispositivoIO->dataHilo.sem_io);
}

void semaforo_signal(t_semaforo* semaforo){
	semaforo->valor++;
	if (semaforo->valor <= 0){
		pcb* procesoBloqueado = queue_pop(semaforo->bloqueados);
		if (procesoBloqueado != NULL) encolarPcbAListos(procesoBloqueado);
    }
}

int semaforo_wait(t_semaforo* semaforo){
	semaforo->valor--;
	if (semaforo->valor < 0){
      return TRUE;
    }
  return FALSE;
}

void semaforoBloquearProceso(t_queue* colaBloqueados, pcb* proceso){
	queue_push(colaBloqueados, proceso);
}

void tratarPcbDeConsolaDesconectada(int pid){
	bool cpuTieneElPidConsola(cpu* unCpu){ return unCpu->pid == pid;}

	cpu* execCPU = list_find(listaCPU, (void*) cpuTieneElPidConsola);

	if(execCPU == NULL){
		printf("Removiendo al proceso #%i porque se desconectó su Consola.\n", pid);
		// Le informo a UMC que libere la memoria asignada al programa:
		int index = pcbListIndex(pid); // indexo el pcb en la lista de procesos
		finalizarPrograma(pid, index);
		// Libero el PCB del proceso y lo saco del sistema:
		liberarPcbNucleo(list_remove(listaProcesos, index));
	}
	else{
		printf("El proceso #%i se removerá cuando finalice ráfaga en CPU #%i porque se desconectó su Consola.\n",
				pid, execCPU->id);
		int * exit_pid = malloc(INT);
		*exit_pid = pid;
		list_add(listaProcesosAbortivos, exit_pid);
	}
}

void verificarDesconexionEnConsolas(){
	int i;
	for (i = 0; i < list_size(listaConsolas); i++){

		consola * unaConsola = (consola *)list_get(listaConsolas, i);
		int fd = unaConsola->fd_consola;

	if (FD_ISSET(fd , &readfds)){ // nuevo mensaje de consola
		int protocolo;
		void * mensaje = NULL;
		mensaje = aplicar_protocolo_recibir(fd, &protocolo);

		if (mensaje == NULL){ // La Consla se desconectó, la quito del sistema y saco su PCB de ejecución:
			log_info(logger,"La Consola #%i se ha desconectado.", unaConsola->id);
			tratarPcbDeConsolaDesconectada(unaConsola->pid);
			cerrarSocket(fd);
			liberarConsola(list_remove(listaConsolas, i));
			return;
		} // fin if msj null
	  } // fin if nuevo msj
	} // fin for consolas
}

void quitarCpuPorSenialSIGUSR1(cpu* unCpu, int index){
	log_info(logger, "El CPU #%i fue quitado del sistema por señal SIGUSR1.", unCpu->id);
	cerrarSocket(unCpu->fd_cpu);
	liberarCPU(list_remove(listaCPU, index));
}

void recorrerListaCPUsYAtenderNuevosMensajes(){

	int i;
	for (i = 0; i < list_size(listaCPU); i++){

		cpu * unCPU = (cpu *)list_get(listaCPU, i);
		int fd = unCPU->fd_cpu;

		bool consolaTieneElPidCPU(consola* unaConsola){ return unaConsola->pid == unCPU->pid;}

		if (FD_ISSET(fd , &readfds)){ // Recibo el nuevo mensaje del CPU:

			int protocolo;
		    void * mensaje = NULL;
		    mensaje = aplicar_protocolo_recibir(fd, &protocolo);

		    if (mensaje == NULL){ // El CPU se desconectó, lo quito del sistema y salvo su PCB:
		    	log_info(logger,"La CPU #%i se ha desconectado. Salvando PCB en ejecución...", unCPU->id);
		    	salvarProcesoEnCPU(unCPU->id);
		    	cerrarSocket(fd);
		    	liberarCPU(list_remove(listaCPU, i));

		    	planificarProceso();
		    	return;

		    }else{
		    	// El mensaje no es NULL, entoces veo de qué se trata:
	switch(protocolo){

	case PCB_FIN_QUANTUM:{

		pcb * pcbEjecutada = (pcb*) mensaje;
		log_info(logger, "Proceso #%i fin de quantum en CPU #%i.", pcbEjecutada->pid, unCPU->id);

		if(envioSenialCPU(unCPU->id)){ quitarCpuPorSenialSIGUSR1(unCPU, i);
		} else { unCPU->disponibilidad = LIBRE; }

		if(seDesconectoConsolaAsociada(pcbEjecutada->pid)) break;

		pcbEjecutada->id_cpu = -1; // le desasigno CPU

		queue_push(colaListos, pcbEjecutada);

		break;
	}
	case PCB_FIN_EJECUCION:{

		int* pidEjecutado = (int*) mensaje;
		log_info(logger, "Proceso #%i fin de ejecución en CPU %i.", *pidEjecutado, unCPU->id);

		if(envioSenialCPU(unCPU->id)){ quitarCpuPorSenialSIGUSR1(unCPU, i);
		} else { unCPU->disponibilidad = LIBRE; }

		if(seDesconectoConsolaAsociada(*pidEjecutado)) break;

		int index = pcbListIndex(*pidEjecutado); // indexo el pcb en la lista de procesos
		// Le informo a UMC que libere la memoria asignada al programa:
		finalizarPrograma(*pidEjecutado, index);
		// Le informo a la Consola asociada:
		bool consolaTieneElPid(consola* unaConsola){ return unaConsola->pid == *pidEjecutado;}
		consola * consolaAsociada = list_remove_by_condition(listaConsolas, (void*) consolaTieneElPid);
		int respuesta = PERMITIDO;
		aplicar_protocolo_enviar(consolaAsociada->fd_consola, FINALIZAR_PROGRAMA, &respuesta);
		// Libero la Consola asociada y la saco del sistema:
		liberarConsola(consolaAsociada);
		// Libero el PCB del proceso y lo saco del sistema:
		liberarPcbNucleo(list_remove(listaProcesos, index));
		free(mensaje);

		break;
	}
	case ENTRADA_SALIDA:{
		// Recibo primero los datos del pedido de IO:
		pedidoIO* datos = (pedidoIO*) mensaje;
		pcb* pcbEjecutada = NULL;
		int head;
		// Espero luego la PCB en ejecución:
		void* entrada = NULL;
		entrada = aplicar_protocolo_recibir(fd, &head);
		if(head == PCB_ENTRADA_SALIDA) pcbEjecutada = (pcb*) entrada;

		if(envioSenialCPU(unCPU->id)){ quitarCpuPorSenialSIGUSR1(unCPU, i);
		} else { unCPU->disponibilidad = LIBRE; }

		if(seDesconectoConsolaAsociada(pcbEjecutada->pid)) break;

		log_info(logger, "Proceso #%i entrada salida en CPU #%i.", pcbEjecutada->pid, unCPU->id);
		pcbEjecutada->id_cpu = -1; // le desasigno CPU

		realizarEntradaSalida(pcbEjecutada, datos);

		break;
	}
	case IMPRIMIR:{

		consola * consolaAsociada = list_find(listaConsolas, (void *)consolaTieneElPidCPU);
		// Le mando el msj a la Consola asociada:
		aplicar_protocolo_enviar(consolaAsociada->fd_consola, IMPRIMIR_TEXTO, string_itoa(*((int*) mensaje)));
		printf("Proceso #%i solicita imprimir variable en CPU #%i.\n", unCPU->pid, unCPU->id);

		break;
	}
	case IMPRIMIR_TEXTO:{

		consola * consolaAsociada = list_find(listaConsolas, (void *)consolaTieneElPidCPU);
		// Le mando el msj a la Consola asociada:
		aplicar_protocolo_enviar(consolaAsociada->fd_consola, IMPRIMIR_TEXTO, mensaje);
		printf("Proceso #%i solicita imprimir texto en CPU #%i.\n", unCPU->pid, unCPU->id);

		break;
	}
	case ABORTO_PROCESO:{ // Es lo mismo que para fin de ejecución

		int* pid = (int*) mensaje;
		int index = pcbListIndex(*pid);
		log_info(logger, "Proceso #%i abortando ejecución en CPU #%i.", *pid, unCPU->id);

		if(envioSenialCPU(unCPU->id)){ quitarCpuPorSenialSIGUSR1(unCPU, i);
		} else { unCPU->disponibilidad = LIBRE; }

		if(seDesconectoConsolaAsociada(*pid)) break;

		// Le informo a UMC que libere la memoria asignada al programa:
		finalizarPrograma(*pid, index);
		// Le informo a la Consola asociada:
		bool consolaTieneElPidProceso(consola* unaConsola){ return unaConsola->pid == *pid;}
		consola * consolaAsociada = list_remove_by_condition(listaConsolas, (void*) consolaTieneElPidProceso);
		int respuesta = NO_PERMITIDO;
		aplicar_protocolo_enviar(consolaAsociada->fd_consola, FINALIZAR_PROGRAMA, &respuesta);
		// Libero la Consola asociada y la saco del sistema:
		liberarConsola(consolaAsociada);
		// Libero el PCB del proceso y lo saco del sistema:
		liberarPcbNucleo(list_remove(listaProcesos, index));
		free(mensaje);

		break;
	}
	case SIGNAL_REQUEST:{

		t_semaforo* semaforo = dictionary_get(diccionarioSemaforos, (char*) mensaje);
		semaforo_signal(semaforo);
		free(mensaje);

		break;
	}
	case WAIT_REQUEST:{

		t_semaforo* semaforo = dictionary_get(diccionarioSemaforos, (char*) mensaje);
		if (semaforo_wait(semaforo)){ // El proceso se bloquea al hacer wait del semáforo
			// Notifico al CPU:
			int* respuesta = malloc(INT);
			*respuesta = CON_BLOQUEO;
			aplicar_protocolo_enviar(fd, RESPUESTA_WAIT, respuesta);
			free(respuesta);
			pcb* waitPcb = NULL;
			int head;
			// Recibo la PCB en ejecución que se bloqueó:
			void* entrada = NULL;
			entrada = aplicar_protocolo_recibir(fd, &head);
			if(head == PCB_WAIT){ waitPcb = (pcb*)entrada; }

			if(envioSenialCPU(unCPU->id)){ quitarCpuPorSenialSIGUSR1(unCPU, i);
			} else { unCPU->disponibilidad = LIBRE; }

			if(seDesconectoConsolaAsociada(waitPcb->pid)) break;

			 semaforoBloquearProceso(semaforo->bloqueados, waitPcb);
		}
		else{ // El proceso no se bloquea y puede seguir ejecutando:
			int* respuesta = malloc(INT);
			*respuesta = SIN_BLOQUEO;
			aplicar_protocolo_enviar(fd, RESPUESTA_WAIT, respuesta);
			free(respuesta);
		}
		free(mensaje);

		break;
	}
	case OBTENER_VAR_COMPARTIDA:{

		// Recibo un char* y devuelvo el int correspondiente:
		var_compartida* varPedida = dictionary_get(diccionarioVarCompartidas, (char*) mensaje);
		aplicar_protocolo_enviar(fd, DEVOLVER_VAR_COMPARTIDA, &(varPedida->valor));
		free(mensaje);

		break;
	}
	case GRABAR_VAR_COMPARTIDA:{ // TODO:  ver memcpy de mensaje, free, si se envia, qué onda

		var_compartida* var_aGrabar = (var_compartida*) mensaje;
		// Actualizo el valor de la variable solicitada:
		var_compartida* varBuscada = dictionary_get(diccionarioVarCompartidas, var_aGrabar->nombre);
		varBuscada->valor = var_aGrabar->valor;
		free(mensaje);
		break;
	}
	case SENIAL_SIGUSR1: {

		printf("Señal SIGUSR1 del CPU #%i. Esperando su PCB en ejecución.\n", unCPU->id);
		list_add(listaCPU_SIGUSR1, &(unCPU->id));
		free(mensaje);

		break;
	}
	default:
		printf("Head #%d de mensaje recibido no reconocido.\n", protocolo);
		break;
	} // fin switch protocolo

	planificarProceso();
	return;
		    } // fin else mensaje not null
		} // fin if nuevo mensaje fd CPU
	} // fin for listaCPU
}

void liberarPcbNucleo(pcb* unPcb){
	free(unPcb->indiceCodigo); unPcb->indiceCodigo = NULL;

	if(unPcb->indiceEtiquetas != NULL){
		free(unPcb->indiceEtiquetas); unPcb->indiceEtiquetas = NULL;
	}

	list_destroy(unPcb->indiceStack); unPcb->indiceStack = NULL;

	free(unPcb); unPcb = NULL;
}

void liberarCPU(cpu * cpu){ free(cpu); cpu = NULL; }

void liberarConsola(consola * consola){ free(consola); consola = NULL;}

void liberarSemaforo(t_semaforo * sem){
	free(sem->nombre); sem->nombre = NULL;
	queue_destroy_and_destroy_elements(sem->bloqueados, (void *) liberarPcbNucleo);
	sem->bloqueados = NULL;
	free(sem); sem = NULL; }

void liberarVarCompartida(var_compartida * var){ free(var->nombre); var->nombre = NULL; free(var); var = NULL; }

void limpiarColecciones(){

	if(list_size(listaProcesos) > 0){
		list_destroy_and_destroy_elements(listaProcesos,(void *) liberarPcbNucleo);
	}
	else{
		list_destroy(listaProcesos);
	}
	listaProcesos = NULL;

	if(list_size(listaCPU) > 0){
		list_destroy_and_destroy_elements(listaCPU,(void *) liberarCPU);
	}
	else{
		list_destroy(listaCPU);
	}
	listaCPU = NULL;

	list_destroy(listaCPU_SIGUSR1);
	listaCPU_SIGUSR1 = NULL;

	list_destroy(listaProcesosAbortivos);
	listaProcesosAbortivos = NULL;

	if(list_size(listaConsolas) > 0){
		list_destroy_and_destroy_elements(listaCPU,(void *) liberarConsola);
	}
	else{
		list_destroy(listaConsolas);
	}
	listaConsolas = NULL;

	if(queue_size(colaListos) > 0){
		queue_destroy_and_destroy_elements(colaListos, (void *) liberarPcbNucleo);
	}
	else{
		queue_destroy(colaListos);
	}
	colaListos = NULL;

	if(dictionary_size(diccionarioSemaforos) > 0){
		dictionary_destroy_and_destroy_elements(diccionarioSemaforos, (void *) liberarSemaforo);
	}
	else{
		dictionary_destroy(diccionarioSemaforos);
	}
	diccionarioSemaforos = NULL;

	if(dictionary_size(diccionarioVarCompartidas) > 0){
		dictionary_destroy_and_destroy_elements(diccionarioVarCompartidas, (void *) liberarVarCompartida);
		}
	else{
		dictionary_destroy(diccionarioVarCompartidas);
	}
	diccionarioVarCompartidas = NULL;

	dictionary_destroy(diccionarioIO);
	diccionarioIO = NULL;
}

void limpiarArchivoConfig(){
	free(config->ioID);
	free(config->ipUMC);
	free(config->retardosIO);
	free(config->semaforosID);
	free(config->semaforosValInicial);
	free(config->variablesCompartidas);
	free(config); config = NULL;
}

void setearValores_config(t_config * archivoConfig){
	return;
}

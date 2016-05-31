#include "fnucleo.h"

// -- CONFIGURACIÓN INCIAL --
void abrirArchivoDeConfiguracion(char * ruta){
	leerArchivoDeConfiguracion(ruta);
	/*if( seteoCorrecto() ){
			log_info(logger, "El archivo de configuración ha sido leído correctamente");
		}*/
}

void setearValores_config(t_config * archivoConfig){
	config = (t_configuracion*)reservarMemoria(sizeof(t_configuracion));

	config->puertoPrograma = config_get_int_value(archivoConfig, "PUERTO_PROG");
	config->puertoCPU = config_get_int_value(archivoConfig, "PUERTO_CPU");
	config->puertoUMC = config_get_int_value(archivoConfig, "PUERTO_UMC");
	config->ipUMC = strdup(config_get_string_value(archivoConfig, "IP_UMC"));
	config->quantum = config_get_int_value(archivoConfig, "QUANTUM");
	config->retardoQuantum = config_get_int_value(archivoConfig, "QUANTUM_SLEEP");
	config->semaforosID = config_get_array_value(archivoConfig, "SEM_IDS");
	config->ioID = config_get_array_value(archivoConfig, "IO_IDS");
	config->variablesCompartidas = config_get_array_value(archivoConfig, "SHARED_VARS");
	config->semaforosValInicial = config_get_array_value(archivoConfig, "SEM_INIT");
	config->retardosIO = config_get_array_value(archivoConfig, "IO_SLEEP");
	config->cantidadPaginasStack = config_get_int_value(archivoConfig, "STACK_SIZE");
}

void inicializarListasYColas(){
	listaCPU = list_create();
	listaConsolas = list_create();
	listaProcesos = list_create();
	colaReady = queue_create();
	colaBlock = queue_create();
}

// --CONEXIONES: CONSOLA(S), UMC Y CPU(S)--
void escucharACPU(){
int puerto = config->puertoCPU;
int newfdCPU, fdEscuchaNucleo, maxfd;

	fdEscuchaNucleo = nuevoSocket();
	asociarSocket(fdEscuchaNucleo, puerto);
	escucharSocket(fdEscuchaNucleo, CONEXIONES_PERMITIDAS);

	fd_set readfds, master;

	FD_ZERO(&readfds);
	FD_ZERO(&master);
	FD_SET(fdEscuchaNucleo, &master);

	maxfd = fdEscuchaNucleo;

	while(TRUE) {

		readfds = master;

		seleccionarSocket(maxfd, &readfds, NULL, NULL, NULL, NULL);

		int i;
		for(i = 0; i <= maxfd; i++) { // recorro los fds buscando si alguno se modificó

			if( FD_ISSET(i, &readfds) ) { // i está modificado. Si no está modificado no se le hace nada

				if(i == fdEscuchaNucleo) { // nueva conexión

					int ret_handshake = 0;

					while(ret_handshake == 0) { // Mientras que no acepte la conexión, por error o inválida
						newfdCPU = aceptarConexionSocket(fdEscuchaNucleo);
						if (validar_conexion(newfdCPU, 0) == FALSE) {
							continue;
						} else {
							ret_handshake = handshake_servidor(newfdCPU, "N");
						}
					} // - conexión válida de handshake -

					FD_SET(newfdCPU, &master);
					if(newfdCPU> maxfd) maxfd = newfdCPU;

					cpu * nuevoCPU = malloc(sizeof(cpu));
					nuevoCPU->fd_cpu = newfdCPU;
					nuevoCPU->disponibilidad = LIBRE;
					list_add(listaCPU, nuevoCPU);
					log_info(logger,"Se ha conectado el CPU con fd: %d", nuevoCPU->fd_cpu);
					planificarProceso();

				} else { // si no es una nueva conexión entonces es un nuevo mensaje
					int head, tamanioMensaje;
					cpu * unCPUActivo = (cpu *)list_get(listaCPU, i);
					void * mensaje = aplicar_protocolo_recibir(unCPUActivo->fd_cpu, &head, &tamanioMensaje);

					 if (mensaje == NULL) { // desconexión o error
						cerrarSocket(unCPUActivo->fd_cpu);
					log_info(logger,"La CPU con fd %d se ha desconectado", unCPUActivo->fd_cpu);
					free(list_remove(listaCPU, unCPUActivo->fd_cpu));
						FD_CLR(unCPUActivo->fd_cpu, &master);
						break;

	} else { // se leyó correctamente el mensaje
		switch(head){
			case ENVIAR_PCB:
			{
			pcb * pcbNuevo=(pcb *) mensaje; // Recibo la PCB actualizada del CPU
			log_info(logger, "Fin de quantum de CPU #%d - Proceso #%d", unCPUActivo->fd_cpu, pcbNuevo->pid);
			actualizarDatosDePCBEjecutada(unCPUActivo, pcbNuevo);
			planificarProceso();
			break;
			}
			case FIN_QUANTUM:
			{
				// completar
			}
									            	}
			printf("CPU #%d: %d\n", unCPUActivo->fd_cpu, head);
					}
				} // - conexión nueva - else - mensaje nuevo -
			} // - i está modificado -
		} // - recorrido de fds -
	} // - while(true) -
	cerrarSocket(fdEscuchaNucleo);
}

void escucharAConsola(){
int puerto = config->puertoPrograma;

int newfd, escuchaNucleo, maxfd;

	escuchaNucleo = nuevoSocket();
	asociarSocket(escuchaNucleo, puerto);
	escucharSocket(escuchaNucleo, CONEXIONES_PERMITIDAS);

	fd_set readfd, master;

	FD_ZERO(&readfd);
	FD_ZERO(&master);
	FD_SET(escuchaNucleo, &master);

	maxfd = escuchaNucleo;

	while(TRUE) {

		readfd = master;

		seleccionarSocket(maxfd, &readfd, NULL, NULL, NULL, NULL);

		int i;
		for(i = 0; i <= maxfd; i++) { // recorro los fds buscando si alguno se modificó

			if( FD_ISSET(i, &readfd) ) { // i está modificado. Si no está modificado no se le hace nada

				if(i == escuchaNucleo) { // nueva conexión

					int ret_handshake = 0;

					while(ret_handshake == 0) { // Mientras que no acepte la conexión, por error o inválida
						newfd = aceptarConexionSocket(escuchaNucleo);
						if (validar_conexion(newfd, 0) == FALSE) {
							continue;
						} else {
							ret_handshake = handshake_servidor(newfd, "N");
						}
					} // - conexión válida de handshake -

					FD_SET(newfd, &master);
					if(newfd > maxfd) maxfd = newfd;

		} else { // si no es una nueva conexión entonces es un nuevo mensaje
			int tamanioMensaje;
			consola * unaConsolaActiva = (consola *)list_get(listaConsolas, i);

			void * scriptRecibido = aplicar_protocolo_recibir(i, ENVIAR_SCRIPT, &tamanioMensaje);

		 if (scriptRecibido == NULL){ // desconexión o error
	// Fin de Consola -> Fin del programa
		log_info(logger, "Se desconectó la Consola con fd #%d. Finalizando el programa con pid #%d.",
					unaConsolaActiva->fd_consola, unaConsolaActiva->pid);

				finalizarPrograma(unaConsolaActiva->pid);
				cerrarSocket(unaConsolaActiva->fd_consola);
				FD_CLR(unaConsolaActiva->fd_consola, &master);
			break;
	} else { // se leyó correctamente el mensaje
		// Recibo el programa de la Consola
			t_string * scriptNuevo = (t_string*)scriptRecibido;
			unaConsolaActiva->programa.tamanio = scriptNuevo->tamanio;
			unaConsolaActiva->programa.texto = strdup(scriptNuevo->texto);

			pcb * nuevoPCB = crearPCB(*scriptNuevo);

			if(nuevoPCB!=NULL){
			unaConsolaActiva->pid = nuevoPCB->pid;

			list_add(listaProcesos, nuevoPCB);
			free(nuevoPCB);
					}
	else{
		aplicar_protocolo_enviar(unaConsolaActiva->fd_consola, RECHAZAR_PROGRAMA, NULL, 0);
	// La saco de mi lista de consolas activas:
		cerrarSocket(unaConsolaActiva->fd_consola);
		FD_CLR(unaConsolaActiva->fd_consola, &master);
	}
		free(scriptNuevo);
		break;
	}
	 free(scriptRecibido);
				} // - conexión nueva - else - mensaje nuevo -
			} // - i está modificado -
		} // - recorrido de fds -
	} // - while(true) -
	cerrarSocket(escuchaNucleo);
}

void conectarConUMC(){
	fd_clienteUMC = nuevoSocket();
	int ret = conectarSocket(fd_clienteUMC, config->ipUMC, config->puertoUMC);
	validar_conexion(ret, 1); // al ser cliente es terminante
	handshake_cliente(fd_clienteUMC, "N");

	int * tamPagina = (int*)malloc(INT);
	recibirPorSocket(fd_clienteUMC, tamPagina, INT);
	tamanioPagina = *tamPagina; // setea el tamaño de pág. que recibe de UMC

	cerrarSocket(fd_clienteUMC);
}

// --LOGGER--
void crearLogger(){
	char * archivoLogNucleo = strdup("NUCLEO_LOG.log");
	logger = log_create("NUCLEO_LOG.log", archivoLogNucleo, TRUE, LOG_LEVEL_INFO);
	free(archivoLogNucleo);
	archivoLogNucleo = NULL;
}

// PROCESOS - PCB
pcb * crearPCB(t_string programa){
	pcb * nuevoPcb = malloc(sizeof(pcb));

	nuevoPcb->pid = asignarPid(listaProcesos);
	nuevoPcb->fdCPU = -1;
	nuevoPcb->paginas_codigo = (programa.tamanio)/tamanioPagina; // acota la división a int
	nuevoPcb->estado = READY;
	nuevoPcb->quantum = config->quantum; // TODO: provisorio, ver manejo CPU

	t_metadata_program* infoProg;
	const char* codigo = strdup(programa.texto);
	infoProg = metadata_desde_literal(codigo);

	nuevoPcb->pc = infoProg->instruccion_inicio + 1; // la siguiente al begin

	// Inicializo los tres índices:
	inicializarIndices(nuevoPcb, infoProg);

	iniciar_programa_t* nuevoPrograma = (iniciar_programa_t*)malloc(sizeof(iniciar_programa_t));
	nuevoPrograma->paginas = config->cantidadPaginasStack + nuevoPcb->paginas_codigo;
	nuevoPrograma->pid = nuevoPcb->pid;
	nuevoPrograma->codigo.texto = strdup(programa.texto);
	nuevoPrograma->codigo.tamanio = programa.tamanio;

	// Le solicito a UMC espacio para el heap del programa y actúo en consecuencia:
	log_info(logger,"Solicitando segmentos de Código y de Stack a UMC para el PID #%d",nuevoPcb->pid);
	aplicar_protocolo_enviar(fd_clienteUMC, INICIAR_PROGRAMA, nuevoPrograma, SIZE_MSG);

	respuestaInicioPrograma* respuestaInicio = (respuestaInicioPrograma*)malloc(sizeof(respuestaInicioPrograma));
	respuestaInicio = aplicar_protocolo_recibir(fd_clienteUMC, INICIAR_PROGRAMA, SIZE_MSG);

	if(respuestaInicio->estadoDelHeap == CREADO){
		log_info(logger,"Se pudo alocar todos los segmentos para el proceso #%d",nuevoPcb->pid);

		free(infoProg);
		free(nuevoPrograma);
		return nuevoPcb;
	}
	else{
		 log_info(logger,
	"No se pudo alocar todos los segmentos para el proceso #%d. Rechazando ingreso al sistema.",
							nuevoPcb->pid);

		 free(infoProg);
		 free(nuevoPrograma);
		 liberarPcb(nuevoPcb);
		 return NULL;
	}
}

void inicializarIndices(pcb* pcb, t_metadata_program* metaData){
	pcb->indiceCodigo.tamanio = 2 * INT * metaData->instrucciones_size;
	pcb->indiceCodigo.instrucciones = metaData->instrucciones_serializado;

	pcb->indiceEtiquetas.tamanio = metaData->etiquetas_size;
	pcb->indiceEtiquetas.etiquetas = metaData->etiquetas;

	pcb->indiceStack.tamanio = config->cantidadPaginasStack * tamanioPagina;
}

int asignarPid(t_list procesos){
	int randomPid = rand() % 1000; // número aleatorio entre 0 y 1000
	int index;
	while ( buscarProcesoPorPid(randomPid, &index) != NULL){
		randomPid = rand() % 1000;
	}
	return randomPid;
}

pcb * buscarProcesoPorPid(int pid, int* index){
	int i;
	pcb * unPcb;
	for (i = 0; i < list_size(listaProcesos); i++){
		unPcb = (pcb *)list_get(listaProcesos, i);
		if(unPcb->pid == pid){
			*index = i;
			return unPcb; // la pcb del proceso es unPcb
		}
	}
	return NULL; // no se encontró el proceso
}

void liberarPCB(pcb * pcb){
	// borrarle todos los campos
	free(pcb);
	pcb = NULL;
}

void limpiarListasYColas(){
	list_destroy_and_destroy_elements(listaProcesos,(void *) liberarPCB );
	listaProcesos = NULL;

	void liberarCPU(cpu * cpu){ free(cpu); cpu = NULL; }
	list_destroy_and_destroy_elements(listaCPU,(void *) liberarCPU );
	listaCPU = NULL;

	void liberarConsola(consola * consola){ free(consola->programa.texto);
	consola->programa.texto = NULL; free(consola); consola = NULL; }
	list_destroy_and_destroy_elements(listaCPU,(void *) liberarConsola );
	listaConsolas = NULL;

	queue_destroy(colaReady);
	queue_destroy(colaBlock);
}

void liberarTodaLaMemoria(){
	limpiarListasYColas();

	log_destroy(logger);
	logger = NULL;
}

// -- PLANIFICACIÓN --
void ejecutarPrograma(t_string programa){
	pcb * nuevoPcb = crearPCB(programa);
	list_add(listaProcesos, nuevoPcb);

	queue_push(colaReady, nuevoPcb);

	planificarProceso();
}

void planificarProceso(){
		int i, asignado = 0;
		for (i = 0; (i < list_size(listaCPU) && asignado==0); i++){
			cpu * unCPU = (cpu *)list_get(listaCPU, i);
			if (unCPU->disponibilidad==LIBRE){
				if (list_size(colaReady->elements) > 0){
					pcb * unPCB = (pcb *)queue_pop(colaReady);
					unPCB->estado = EXEC;
					unPCB->fdCPU = unCPU->fd_cpu;
					unCPU->disponibilidad = OCUPADO;

					aplicar_protocolo_enviar(unCPU->fd_cpu, ENVIAR_PCB, unPCB, SIZE_MSG);

					asignado = 1;
				}
			}
		}
	}

void finalizarPrograma(int pid){
	int index;
	pcb * procesoAFinalizar = buscarProcesoPorPid(pid, &index);
	if (procesoAFinalizar!=NULL){

	log_info(logger,"Finalizando el proceso con PID #%d.",pid);

	int* _pid = (int*)malloc(INT);
	*_pid = pid;
// Aviso a UMC que libere la memoria asignada al proceso:
	aplicar_protocolo_enviar(fd_clienteUMC, FINALIZAR_PROGRAMA, _pid, INT);
		// remover programa
	}
}

void actualizarDatosDePCBEjecutada(cpu * unCPU, pcb * pcbNuevo){
	unCPU->disponibilidad = LIBRE;
	int index;
	pcb * unPcb = buscarProcesoPorPid(pcbNuevo->pid, &index);

	unPcb->estado = pcbNuevo->estado;
	unPcb->fdCPU = -1;

		switch (unPcb->estado) {
			case EXEC:
				unPcb->estado = READY;
				 queue_push(colaReady, unPcb);
				break;
			case BLOCK:
				 queue_push(colaBlock, unPcb);
				break;
			case EXIT:
				log_info(logger, "El proceso con PID #%d ha finalizado", unPcb->pid);
				liberarPcb(unPcb);
				free(list_remove(listaProcesos, index));
				break;
		}
		liberarPcb(unPcb);
}


// --FUNCIONES AUXILIARES--

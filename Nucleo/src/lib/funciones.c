#include "funciones.h"

/*******************************
 *    FUNCIONES SECUNDARIAS    *
 ******************************/
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

void crearLogger(){
	char * archivoLogNucleo = strdup("NUCLEO_LOG.log");
	logger = log_create("NUCLEO_LOG.log", archivoLogNucleo, TRUE, LOG_LEVEL_INFO);
	free(archivoLogNucleo);
	archivoLogNucleo = NULL;
}

int solicitarSegmentosAUMC(pcb * nuevoPcb, string programa){
	nuevoPcb->pid = asignarPid(listaProcesos);

	int aux_div = programa.tamanio / tamanioPagina;
	int resto_div = programa.tamanio % tamanioPagina;
	nuevoPcb->paginas_codigo = (resto_div==0)?aux_div:aux_div+1;

	// Le solicito a UMC espacio para el heap del programa y actúo en consecuencia:
		inicioPrograma* solicitudDeInicio = (inicioPrograma*)malloc(sizeof(inicioPrograma));
		solicitudDeInicio->paginas = config->cantidadPaginasStack + nuevoPcb->paginas_codigo;
		solicitudDeInicio->pid = nuevoPcb->pid;
		solicitudDeInicio->contenido = strdup(programa.cadena);

		printf("Solicitando segmentos de código y de stack a UMC para el Proceso #%d", nuevoPcb->pid);
		aplicar_protocolo_enviar(fd_UMC, INICIAR_PROGRAMA, solicitudDeInicio);
		free(solicitudDeInicio->contenido);
		free(solicitudDeInicio);

		respuestaInicioPrograma* respuestaInicio = NULL;
		recibirYAsignarPaquete(fd_UMC, INICIAR_PROGRAMA, respuestaInicio);

			if(respuestaInicio->estadoDelHeap == CREADO){
				printf("Se pudo alocar todos los segmentos para el Proceso #%d", nuevoPcb->pid);

				free(respuestaInicio);

				return TRUE;
				}
			else{
				printf("UMC no pudo alocar segmentos para el Proceso #%d. Rechazando ingreso al sistema.",
						nuevoPcb->pid);

				free(respuestaInicio);

				return FALSE;
				}
}



pcb * crearPcb(string programa){
	pcb * nuevoPcb = malloc(sizeof(pcb));

	int respuestaUMC = solicitarSegmentosAUMC(nuevoPcb, programa);

	if(respuestaUMC == FALSE){
		return NULL;
	}
	else{
			nuevoPcb->estado = READY;
			nuevoPcb->id_cpu = -1;
			// Ahora, analizo con el parser el código del programa para obtener su metadata:
			char* codigo = malloc(programa.tamanio);
			codigo = strdup(programa.cadena);
			t_metadata_program* infoProg = metadata_desde_literal((const char*) codigo);
			free(codigo);

			nuevoPcb->pc = infoProg->instruccion_inicio + 1; // la siguiente al begin

			// Inicializo los tres índices:
			inicializarIndices(nuevoPcb, infoProg);
			metadata_destruir(infoProg);

			return nuevoPcb;
	}
}

void inicializarIndices(pcb* pcb, t_metadata_program* metaData){
	pcb->tamanioIndiceCodigo = 2 * INT * metaData->instrucciones_size;
	pcb->indiceCodigo = metaData->instrucciones_serializado;

	pcb->tamanioIndiceEtiquetas = metaData->etiquetas_size;
	pcb->indiceEtiquetas = metaData->etiquetas;

	pcb->tamanioIndiceStack = config->cantidadPaginasStack * tamanioPagina;
	pcb->indiceStack = NULL;
}

int asignarPid(t_list * procesos){
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

void salvarProcesoEnCPU(int id_cpu){
	bool esLaPcbEnEjecucion(pcb * pcb){ return pcb->id_cpu == id_cpu; }

	pcb * unPcb = (pcb *) list_find(listaProcesos, (void *) esLaPcbEnEjecucion);

	if(unPcb != NULL){
		unPcb->id_cpu = -1;
		unPcb->estado = READY;

		queue_push(colaListos, unPcb);

		planificarProceso();
	}
	// TODO: ver qué hacer si no está en la cola
}

void liberarPcb(pcb * pcb){
	free(pcb->indiceCodigo);
	free(pcb->indiceEtiquetas);
	free(pcb->indiceStack->listaPosicionesArgumentos);
	free(pcb->indiceStack->listaVariablesLocales);
	free(pcb->indiceStack);

	free(pcb);
	pcb = NULL;
}

void liberarCPU(cpu * cpu){ free(cpu); cpu = NULL; }
void liberarConsola(consola * consola){ free(consola->programa.cadena);
	consola->programa.cadena = NULL; free(consola); consola = NULL; }

void limpiarListasYColas(){
	list_destroy_and_destroy_elements(listaProcesos,(void *) liberarPcb );
	listaProcesos = NULL;

	list_destroy_and_destroy_elements(listaCPU,(void *) liberarCPU );
	listaCPU = NULL;

	list_destroy_and_destroy_elements(listaCPU,(void *) liberarConsola );
	listaConsolas = NULL;

	queue_destroy(colaListos);
	queue_destroy(colaBloqueados);
	colaListos = NULL;
	colaBloqueados = NULL;
}

void limpiarArchivoConfig(){
	free(config->ioID);
	free(config->ipUMC);
	free(config->retardosIO);
	free(config->semaforosID);
	free(config->semaforosValInicial);
	free(config->variablesCompartidas);
	free(config);
	config = NULL;
}

void planificarProceso(){
	int i, asignado = 0;
		for (i = 0; (i < list_size(listaCPU) && asignado==0); i++){
			cpu * unCPU = (cpu *)list_get(listaCPU, i);
			if (unCPU->disponibilidad == LIBRE){
				if (queue_size(colaListos) > 0){
					pcb * unPCB = (pcb *)queue_pop(colaListos);
					unPCB->estado = EXEC;
					unPCB->id_cpu = unCPU->id;
					unCPU->disponibilidad = OCUPADO;

		aplicar_protocolo_enviar(unCPU->fd_cpu, PCB, unPCB);

		log_info(logger, "El proceso %i ingresa a ejecución en CPU %i.", unPCB->pid, unCPU->id);

		asignado = 1;
				}
			}
		}
	}

void finalizarPrograma(int pid){
	int index;
	pcb * procesoAFinalizar = buscarProcesoPorPid(pid, &index);
	if (procesoAFinalizar!=NULL){

	log_info(logger,"Liberarndo la memoria asignada al Proceso #%d.", pid);

	// Aviso a UMC que libere la memoria asignada al proceso:
	int* pidAFinalizar = (int*)malloc(INT);
	*pidAFinalizar = pid;

	aplicar_protocolo_enviar(fd_UMC, FINALIZAR_PROGRAMA, pidAFinalizar);
	free(pidAFinalizar);
	}
}

void actualizarDatosDePcbEjecutada(cpu * unCPU, pcb * pcbNuevo){
	unCPU->disponibilidad = LIBRE;

		int index;
		pcb * unPcb = buscarProcesoPorPid(pcbNuevo->pid, &index);

		unPcb->estado = pcbNuevo->estado;
		unPcb->id_cpu = -1;

		switch (unPcb->estado) {
			case EXEC:
				unPcb->estado = READY;
				queue_push(colaListos, unPcb);
				break;
			case BLOCK:
				// Manejar I/O
				queue_push(colaBloqueados, unPcb);
				break;
			case EXIT:
				log_info(logger, "El Programa #%i ha finalizado.", unPcb->pid);

				// Le informo a UMC:
				finalizarPrograma(unPcb->pid);

				bool consolaTieneElPid(void* unaConsola){
					return (((consola*) unaConsola)->pid) == unPcb->pid;}
				consola * consolaAsociada = list_remove_by_condition(listaConsolas, consolaTieneElPid);

				// Le informo a la Consola asociada:
				aplicar_protocolo_enviar(consolaAsociada->fd_consola, FINALIZAR_PROGRAMA, NULL);
				liberarConsola(consolaAsociada);

				// Libero el PCB del proceso:
				liberarPcb(list_remove(listaProcesos, index));

				break;
		}
		liberarPcb(pcbNuevo);
}

void manejarES(){
	/*if (!queue_is_empty(colaBloqueados)){
		pcb * unPcb = queue_pop(colaBloqueados);

		*** MANEJAR ENTRADA/SALIDA ***

		if( *** realizoEntradaSalida *** ){

			unPcb->estado=LISTO;
			queue_push(colaListos, unPcb);
			planificarProceso();

		}
	}*/
}

void notificarCambioDelQuantumACPU(){

	info_quantum * infoQuantum = (info_quantum*) malloc(sizeof(info_quantum));
		infoQuantum->quantum = config->quantum;
		infoQuantum->retardoQuantum = config->retardoQuantum;

	void enviarNuevoQuantum(cpu * unCpu){aplicar_protocolo_enviar(unCpu->fd_cpu, QUANTUM_MODIFICADO, infoQuantum);}
	list_iterate(listaCPU, (void *)enviarNuevoQuantum);

	free(infoQuantum);
}

void detectarCambiosEnArchivoConfig(){
	char buffer[EVENT_SIZE];

		fd_inotify = inotify_init();
		if (fd_inotify < 0) {
			manejarError("inotify_init");
		}

		int watch_descriptor = inotify_add_watch(fd_inotify, RUTA_CONFIG_NUCLEO, IN_MODIFY);

		int length = read(fd_inotify, buffer, EVENT_SIZE);
		if (length < 0) {
			manejarError("read");
		}

		int offset = 0;

		while (offset < length) {

			struct inotify_event *event = (struct inotify_event *) &buffer[offset];

			if (event->len) {

				if (event->mask & IN_MODIFY) {
					if (event->mask & IN_ISDIR) {
						printf("El directorio %s fue modificado.\n", event->name);
					} else {
						printf("EL archivo %s fue modificado.\n", event->name);

						notificarCambioDelQuantumACPU();
					}
				}
			}
			offset += sizeof (struct inotify_event) + event->len;
		}

		inotify_rm_watch(fd_inotify, watch_descriptor);
		close(fd_inotify);
}

// TODO: METER EN EL SELECT EL FD INOTIFY

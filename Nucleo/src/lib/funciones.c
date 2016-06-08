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

pcb * crearPcb(string programa){
	pcb * nuevoPcb = malloc(sizeof(pcb));

	nuevoPcb->pid = asignarPid(listaProcesos);
	nuevoPcb->id_cpu = -1;

	int aux_div = programa->tamanio / tamanioPagina;
	int resto_div = programa->tamanio % tamanioPagina;
	nuevoPcb->paginas_codigo = (resto_div==0)?aux_div:aux_div+1;

	nuevoPcb->estado = READY;
	nuevoPcb->quantum = config->quantum;

	// Ahora, analizo con el parser el código del programa para obtener su metadata:
	const char* codigo = strdup(programa.cadena);
	t_metadata_program* infoProg = metadata_desde_literal(codigo);
	// free(codigo);

	nuevoPcb->pc = infoProg->instruccion_inicio + 1; // la siguiente al begin

	// Inicializo los tres índices:
	inicializarIndices(nuevoPcb, infoProg);
	liberarMetadataDelPrograma(infoProg);

	// Le solicito a UMC espacio para el heap del programa y actúo en consecuencia:
	inicioPrograma* solicitudDeInicio = (inicioPrograma*)malloc(sizeof(inicioPrograma));
	solicitudDeInicio->paginas = config->cantidadPaginasStack + nuevoPcb->paginas_codigo;
	solicitudDeInicio->pid = nuevoPcb->pid;
	solicitudDeInicio->codigo.cadena = strdup(programa.cadena);
	solicitudDeInicio->codigo->tamanio = programa->tamanio;

	printf("Solicitando segmentos de código y de stack a UMC para el Proceso #%d", nuevoPcb->pid);
	aplicar_protocolo_enviar(fd_UMC, INICIAR_PROGRAMA, solicitudDeInicio);

	respuestaInicioPrograma* respuestaInicio = (respuestaInicioPrograma*)aplicar_protocolo_recibir(fd_UMC, INICIAR_PROGRAMA);

	free(solicitudDeInicio->codigo.cadena);
	free(solicitudDeInicio);

	if(respuestaInicio->estadoDelHeap == CREADO){
		log_info(logger,"Se pudo alocar todos los segmentos para el Proceso #%d", nuevoPcb->pid);

		free(respuestaInicio);

		return nuevoPcb;
	}
	else{
		 log_info(logger,
	"UMC no pudo alocar segmentos para el Proceso #%d. Rechazando ingreso al sistema.", nuevoPcb->pid);

		 free(respuestaInicio);

		 return NULL;
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

void liberarMetadataDelPrograma(t_metadata_program * metadata){
	// completar
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

void salvarProcesoEnCPU(int CPU){
	bool cpuActualDelPcb(pcb * pcb){ return pcb->id_cpu == CPU; }

	pcb * unPcb = (pcb *) list_find(listaProcesos, (void *) cpuActualDelPcb);

	if(unPcb != NULL){
		unPcb->id_cpu = -1;
		unPcb->estado = READY;

		queue_push(colaListos, unPcb);

		planificarProceso();
	}
	// TODO: ver qué hacer si no está en la cola
}

void liberarPcb(pcb * pcb){
	// borrarle los campos necesarios

	free(pcb);
	pcb = NULL;
}

void limpiarListasYColas(){
	list_destroy_and_destroy_elements(listaProcesos,(void *) liberarPcb );
	listaProcesos = NULL;

	void liberarCPU(cpu * cpu){ free(cpu); cpu = NULL; }
	list_destroy_and_destroy_elements(listaCPU,(void *) liberarCPU );
	listaCPU = NULL;

	void liberarConsola(consola * consola){ free(consola->programa.cadena);
	consola->programa.cadena = NULL; free(consola); consola = NULL; }
	list_destroy_and_destroy_elements(listaCPU,(void *) liberarConsola );
	listaConsolas = NULL;

	queue_destroy(colaListos);
	queue_destroy(colaBloqueados);
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

	log_info(logger,"Finalizando el Proceso #%d.", pid);

	// Aviso a UMC que libere la memoria asignada al proceso:
	int* pidAFinalizar = (int*)malloc(INT);
	*pidAFinalizar = pid;

	aplicar_protocolo_enviar(fd_UMC, FINALIZAR_PROGRAMA, pidAFinalizar);
	free(pidAFinalizar);
		// remover programa
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
				queue_push(colaBloqueados, unPcb);
				break;
			case EXIT:
				log_info(logger, "El Programa %i ha finalizado.", unPcb->pid);
				liberarPcb(list_remove(listaProcesos, index));
				break;
		}
		liberarPcb(pcbNuevo);
}


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

pcb * crearPCB(string programa){
	pcb * nuevoPcb = malloc(sizeof(pcb));

	nuevoPcb->pid = asignarPid(*listaProcesos);
	nuevoPcb->fdCPU = -1;
	nuevoPcb->paginas_codigo = (programa.tamanio)/tamanioPagina; // acota la división a int
	nuevoPcb->estado = READY;
	nuevoPcb->quantum = config->quantum; // TODO: provisorio, ver manejo CPU

	t_metadata_program* infoProg;
	const char* codigo = strdup(programa.cadena);
	infoProg = metadata_desde_literal(codigo);

	nuevoPcb->pc = infoProg->instruccion_inicio + 1; // la siguiente al begin

	// Inicializo los tres índices:
	inicializarIndices(nuevoPcb, infoProg);

	inicioPrograma* nuevoPrograma = (inicioPrograma*)malloc(sizeof(inicioPrograma));
	nuevoPrograma->paginas = config->cantidadPaginasStack + nuevoPcb->paginas_codigo;
	nuevoPrograma->pid = nuevoPcb->pid;
	nuevoPrograma->codigo.cadena = strdup(programa.cadena);
	nuevoPrograma->codigo.tamanio = programa.tamanio;

	// Le solicito a UMC espacio para el heap del programa y actúo en consecuencia:
	log_info(logger,"Solicitando segmentos de Código y de Stack a UMC para el PID #%d",nuevoPcb->pid);
	aplicar_protocolo_enviar(fd_clienteUMC, INICIAR_PROGRAMA, nuevoPrograma);

	respuestaInicioPrograma* respuestaInicio = (respuestaInicioPrograma*)malloc(sizeof(respuestaInicioPrograma));
	respuestaInicio = aplicar_protocolo_recibir(fd_clienteUMC, INICIAR_PROGRAMA);

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

	queue_destroy(colaReady);
	queue_destroy(colaBlock);
}

void ejecutarPrograma(string programa){
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

					aplicar_protocolo_enviar(unCPU->fd_cpu, PCB, unPCB);

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
	aplicar_protocolo_enviar(fd_clienteUMC, FINALIZAR_PROGRAMA, _pid);
	free(_pid);
		// remover programa
	}
}

void actualizarDatosDePcbEjecutada(cpu * unCPU, pcb * pcbNuevo){
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


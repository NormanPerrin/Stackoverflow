#include "funciones.h"

/*******************************
 *    FUNCIONES SECUNDARIAS    *
 ******************************/
int setearValoresDeConfig(t_config * archivoConfig){

	config = (t_configuracion*)malloc(sizeof(t_configuracion));

		if (config_has_property(archivoConfig, "PUERTO_UMC")) {
			config->puertoUMC = config_get_int_value(archivoConfig, "PUERTO_UMC");
		} else {
			log_info(logger, "El archivo de configuracion no contiene la clave PUERTO_UMC.\n");
			return FALSE;
		}
		if (config_has_property(archivoConfig, "IP_UMC")) {
			config->ipUMC = strdup(config_get_string_value(archivoConfig, "IP_UMC"));
		} else {
			log_info(logger, "El archivo de configuracion no contiene la clave IP_UMC.\n");
			return FALSE;
		}
		if (config_has_property(archivoConfig, "PUERTO_PROG")) {
			config->puertoPrograma = config_get_int_value(archivoConfig, "PUERTO_PROG");
		} else {
			log_info(logger, "El archivo de configuracion no contiene la clave PUERTO_PROG.\n");
			return FALSE;
		}
		if (config_has_property(archivoConfig, "PUERTO_CPU")) {
			config->puertoCPU = config_get_int_value(archivoConfig, "PUERTO_CPU");
		} else {
			log_info(logger, "El archivo de configuracion no contiene la clave PUERTO_CPU.\n");
			return FALSE;
		}
		if (config_has_property(archivoConfig, "QUANTUM")) {
			config->quantum = config_get_int_value(archivoConfig, "QUANTUM");
		} else {
			log_info(logger, "El archivo de configuracion no contiene la clave QUANTUM.\n");
			return FALSE;
		}
		if (config_has_property(archivoConfig, "QUANTUM_SLEEP")) {
			config->retardoQuantum = config_get_int_value(archivoConfig, "QUANTUM_SLEEP");
		} else {
			log_info(logger, "El archivo de configuracion no contiene la clave QUANTUM_SLEEP.\n");
			return FALSE;
		}
		if (config_has_property(archivoConfig, "SEM_IDS")) {
			config->semaforosID = config_get_array_value(archivoConfig, "SEM_IDS");
		} else {
			log_info(logger, "El archivo de configuracion no contiene la clave SEM_IDS.\n");
			return FALSE;
		}
		if (config_has_property(archivoConfig, "SEM_INIT")) {
			config->semaforosValInicial = config_get_array_value(archivoConfig, "SEM_INIT");
		} else {
			log_info(logger, "El archivo de configuracion no contiene la clave SEM_INIT.\n");
			return FALSE;
		}
		if (config_has_property(archivoConfig, "IO_IDS")) {
			config->ioID = config_get_array_value(archivoConfig, "IO_IDS");
		} else {
			log_info(logger, "El archivo de configuracion no contiene la clave IO_IDS");
			return FALSE;
		}
		if (config_has_property(archivoConfig, "IO_SLEEP")) {
			config->retardosIO = config_get_array_value(archivoConfig, "IO_SLEEP");
		} else {
			log_info(logger, "El archivo de configuracion no contiene la clave IO_SLEEP");
			return FALSE;
		}
		if (config_has_property(archivoConfig, "SHARED_VARS")) {
			config->variablesCompartidas = config_get_array_value(archivoConfig, "SHARED_VARS");
		} else {
			log_info(logger, "El archivo de configuracion no contiene la clave SHARED_VARS.\n");
			return FALSE;
		}
		if (config_has_property(archivoConfig, "STACK_SIZE")) {
			config->cantidadPaginasStack = config_get_int_value(archivoConfig, "STACK_SIZE");
		} else {
			log_info(logger, "El archivo de configuracion no contiene la clave STACK_SIZE.\n");
			return FALSE;
		}
		config_destroy(archivoConfig); // Libero la estructura archivoConfig

	return TRUE;
}

var_compartida* crearVariableCompartida(char* nombre, int valorInicial){

var_compartida* var = malloc(sizeof(var_compartida));
  var->nombre = strdup(nombre);
  var->valor = valorInicial;

  return var;
}

void registrarSemaforo(char* name, int value){
	t_semaforo* sem = semaforo_create(name, value);
	dictionary_put(diccionarioSemaforos, sem->nombre, sem);
}

void registrarVariableCompartida(char* name, int value){
	var_compartida* var = crearVariableCompartida(name, value);
	dictionary_put(diccionarioVarCompartidas,var->nombre, var);
}

int solicitarSegmentosAUMC(pcb * nuevoPcb, char* programa){
	nuevoPcb->pid = asignarPid(listaProcesos);

	int tam_prog = strlen(programa)+1;
	int aux_div = tam_prog / tamanioPagina;
	int resto_div = tam_prog % tamanioPagina;
	nuevoPcb->paginas_codigo = (resto_div==0)?aux_div:aux_div+1;
	nuevoPcb->paginas_stack = config->cantidadPaginasStack;

	// Le solicito a UMC espacio para el heap del programa y verifico su respuesta:
		inicioPrograma* solicitudDeInicio = (inicioPrograma*)malloc(sizeof(inicioPrograma));
		solicitudDeInicio->paginas = nuevoPcb->paginas_stack + nuevoPcb->paginas_codigo;
		solicitudDeInicio->pid = nuevoPcb->pid;
		solicitudDeInicio->contenido = strdup(programa);

		printf("Solicitando segmentos de código y de stack a UMC para el Proceso #%d.\n", nuevoPcb->pid);
		aplicar_protocolo_enviar(fd_UMC, INICIAR_PROGRAMA, solicitudDeInicio);
		free(solicitudDeInicio->contenido);
		free(solicitudDeInicio);

		int* respuestaUMC = NULL;
		int head;
		void * entrada = NULL;
		entrada = aplicar_protocolo_recibir(fd_UMC, &head);

			if(head == RESPUESTA_INICIO_PROGRAMA)
				respuestaUMC = (int*)entrada;

				if(*respuestaUMC == CREADO){
					printf("UMC pudo alocar todos los segmentos del Proceso #%d.\n", nuevoPcb->pid);
					free(respuestaUMC );
					return TRUE;
				}
				else{
					printf("UMC no pudo alocar los segmentos del Proceso #%d. Rechazando ingreso al sistema.\n",
						nuevoPcb->pid);
					free(respuestaUMC);
					return FALSE;
				}
}

pcb * crearPcb(char* programa){
	pcb * nuevoPcb = malloc(sizeof(pcb));

	int respuestaUMC = solicitarSegmentosAUMC(nuevoPcb, programa);

	if(respuestaUMC == FALSE){
		free(nuevoPcb);
		return NULL;
	}
	else{
		// La UMC pudo alocar los segmentos del proceso, entonces sigo:
			nuevoPcb->id_cpu = -1;
			nuevoPcb->quantum = config->quantum;
			nuevoPcb->quantum_sleep = config->retardoQuantum;

		// Ahora, analizo con el parser el código del programa para obtener su metadata:
			t_metadata_program* infoProg = metadata_desde_literal(programa);

			nuevoPcb->paginaActualCodigo = 0;
			nuevoPcb->primerPaginaStack = nuevoPcb->paginas_codigo;
			nuevoPcb->paginaActualStack = nuevoPcb->primerPaginaStack;
			nuevoPcb->pc = infoProg->instruccion_inicio; // la siguiente al begin
			nuevoPcb->stackPointer = 0; // offset total en memoria
			nuevoPcb->cantidad_instrucciones = infoProg->instrucciones_size;

		// Inicializo índice de código:
			nuevoPcb->tamanioIndiceCodigo = sizeof(t_intructions) * infoProg->instrucciones_size;
			nuevoPcb->indiceCodigo = infoProg->instrucciones_serializado;

		// Inicializo índice de stack:
			nuevoPcb->indiceStack = stack_create();
			nuevoPcb->numeroContextoEjecucionActualStack = 0;

		// Inicializo índice de etiquetas:
			if (infoProg->cantidad_de_etiquetas > 0 || infoProg->cantidad_de_funciones > 0) {
				nuevoPcb->indiceEtiquetas = strdup(infoProg->etiquetas);
				nuevoPcb->tamanioIndiceEtiquetas = infoProg->etiquetas_size;
			} else {
				nuevoPcb->indiceEtiquetas = NULL;
			}

			metadata_destruir(infoProg);

			return nuevoPcb;
	}
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

int pcbListIndex(int pid){
	int i, index;
		pcb * unPcb;
		for (i = 0; i < list_size(listaProcesos); i++){
			unPcb = (pcb *)list_get(listaProcesos, i);
			if(unPcb->pid == pid){
				index = i;
				free(unPcb);
				return index;
			}
		}
		free(unPcb);
		return -1; // no se encontró el proceso
}

void salvarProcesoEnCPU(int id_cpu){
	bool esLaPcbEnEjecucion(pcb * pcb){ return pcb->id_cpu == id_cpu; }

	pcb * unPcb = (pcb *) list_find(listaProcesos, (void *) esLaPcbEnEjecucion);

	if(unPcb != NULL){
		unPcb->id_cpu = -1;

		queue_push(colaListos, unPcb);

		planificarProceso();
	}
	// TODO: ver qué hacer si no está en la cola
}

void liberarCPU(cpu * cpu){ free(cpu); cpu = NULL; }

void liberarConsola(consola * consola){ free(consola->programa);
	consola->programa = NULL; free(consola); consola = NULL; }

void liberarSemaforo(t_semaforo * sem){
	free(sem->nombre);
	queue_destroy_and_destroy_elements(sem->bloqueados, (void *) liberarPcb);
	sem->bloqueados = NULL; free(sem); sem = NULL; }

void liberarVarCompartida(var_compartida * var){ free(var->nombre); free(var); var = NULL; }

void limpiarColecciones(){
	list_destroy_and_destroy_elements(listaProcesos,(void *) liberarPcb);
	listaProcesos = NULL;

	list_destroy_and_destroy_elements(listaCPU,(void *) liberarCPU);
	listaCPU = NULL;

	list_destroy_and_destroy_elements(listaCPU,(void *) liberarConsola);
	listaConsolas = NULL;

	queue_destroy_and_destroy_elements(colaListos, (void *) liberarPcb);
	colaListos = NULL;

	dictionary_destroy_and_destroy_elements(diccionarioSemaforos, (void *) liberarSemaforo);
	diccionarioSemaforos = NULL;

	dictionary_destroy_and_destroy_elements(diccionarioVarCompartidas, (void *) liberarVarCompartida);
	diccionarioVarCompartidas = NULL;
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

	pthread_mutex_lock(&mutex_planificarProceso);

	int i, asignado = 0;
		for (i = 0; (i < list_size(listaCPU) && asignado==0); i++){
			cpu * unCPU = (cpu *)list_get(listaCPU, i);
			if (unCPU->disponibilidad == LIBRE){
				if (queue_size(colaListos) > 0){
					pcb * unPCB = (pcb *)queue_pop(colaListos);
					unPCB->id_cpu = unCPU->id;
					unCPU->disponibilidad = OCUPADO;

		aplicar_protocolo_enviar(unCPU->fd_cpu, PCB, unPCB);

		log_info(logger, "El proceso %i ingresa a ejecución en CPU %i.", unPCB->pid, unCPU->id);

		asignado = 1;
				}
			}
		}
		pthread_mutex_unlock(&mutex_planificarProceso);
	}

void finalizarPrograma(int pid, int index){

	if (index != -1){

	log_info(logger,"Liberarndo la memoria asignada al Proceso #%d.", pid);

	// Aviso a UMC que libere la memoria asignada al proceso:
	int* pidAFinalizar = (int*)malloc(INT);
	*pidAFinalizar = pid;

	aplicar_protocolo_enviar(fd_UMC, FINALIZAR_PROGRAMA, pidAFinalizar);
	free(pidAFinalizar);
	}
}

void actualizarPcbEjecutada(cpu * unCPU, pcb * pcbEjecutada, int estado){

	// El CPU pasa de Ocupada a Libre:
	unCPU->disponibilidad = LIBRE;

	int index = pcbListIndex(pcbEjecutada->pid);

	pcbEjecutada->id_cpu = -1; // El proceso no tiene asignado aún un CPU.

		switch (estado) {

			case PCB_FIN_QUANTUM:{

				queue_push(colaListos, pcbEjecutada);

				break;
			}
			case PCB_FIN_EJECUCION:{

				// Le informo a UMC:
				finalizarPrograma(pcbEjecutada->pid, index);

			// Le informo a la Consola asociada:
			bool consolaTieneElPid(void* unaConsola){ return (((consola*) unaConsola)->pid) == pcbEjecutada->pid;}
			consola * consolaAsociada = list_remove_by_condition(listaConsolas, consolaTieneElPid);

			aplicar_protocolo_enviar(consolaAsociada->fd_consola, FINALIZAR_PROGRAMA, NULL);
			// Libero la Consola asociada y la saco del sistema:
			liberarConsola(consolaAsociada);

			// Libero el PCB del proceso y lo saco del sistema:
			liberarPcb(list_remove(listaProcesos, index));

			break;
			}
		}
		liberarPcb(pcbEjecutada);

		planificarProceso();
}
/*
void manejarES(){
	if (!queue_is_empty(colaBloqueados)){
		pcb * unPcb = queue_pop(colaBloqueados);

		*** MANEJAR ENTRADA/SALIDA ***

		if( *** realizoEntradaSalida *** ){

			unPcb->estado=LISTO;
			queue_push(colaListos, unPcb);
			planificarProceso();

		}
	}
}*/

pcb* copiarPcb(pcb* proceso){
 pcb* copia = malloc(sizeof *copia);
  memcpy(copia, proceso, sizeof *copia);
  return copia;
}

void encolarPcbAListos(pcb* proceso){
  pcb* copia = copiarPcb(proceso);
  log_info(logger,"Moviendo proceso %d a la cola de Listos", proceso->pid);
  queue_push(colaListos, copia);
}

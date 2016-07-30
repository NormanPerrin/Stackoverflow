#include "principalesCPU.h"

int validar_cliente(char *id) { return 0; }

int validar_servidor(char *id) {
	if(!strcmp(id, "U") || !strcmp(id, "N")) {
		printf("Servidor aceptado.\n");
		return TRUE;
	} else {
		printf("Servidor rechazado.\n");
		return FALSE;
	}
}

void crearLoggerCPU(){
	char * archivoLogCPU = strdup("CPU_LOG.log");
	logger = log_create("CPU_LOG.log", archivoLogCPU, TRUE, LOG_LEVEL_INFO);
	free(archivoLogCPU); archivoLogCPU = NULL;
}

void setearValores_config(t_config * archivoConfig){
	config = reservarMemoria(sizeof(t_configuracion));
	config->ipNucleo = strdup(config_get_string_value(archivoConfig, "IP_NUCLEO"));
	config->ipUMC = strdup(config_get_string_value(archivoConfig, "IP_UMC"));
	config->puertoNucleo = config_get_int_value(archivoConfig, "PUERTO_NUCLEO");
	config->puertoUMC = config_get_int_value(archivoConfig, "PUERTO_UMC");
}

void liberarPcbActiva(){

	if(pcbActual->indiceCodigo != NULL){
		free(pcbActual->indiceCodigo); pcbActual->indiceCodigo = NULL;
	}

	if(pcbActual->indiceEtiquetas != NULL){
		free(pcbActual->indiceEtiquetas); pcbActual->indiceEtiquetas = NULL;
	}

	if(pcbActual->indiceStack != NULL){
		if(list_size(pcbActual->indiceStack) > 0){
			int i;
			for(i=0; i<list_size(pcbActual->indiceStack); i++){
				registroStack* reg = (registroStack*) list_remove(pcbActual->indiceStack, i);
				if(reg != NULL){
					liberarRegistroStack(reg);
				}
			}
		}
		free(pcbActual->indiceStack); pcbActual->indiceStack = NULL;
	}

	if(pcbActual != NULL){
		free(pcbActual); pcbActual = NULL;
	}
}

void atenderSenialSIGUSR1(){
		log_info(logger, "Se recibió señal SIGUSR1. Notificando a Núcleo.");
		int* info_senial = malloc(INT);
		* info_senial = SENIAL;
		aplicar_protocolo_enviar(fdNucleo, SENIAL_SIGUSR1, info_senial);
		free(info_senial); info_senial = NULL;

		if (cpuOciosa){ // cpu sale del sistema
			exitCPU();
			exit(EXIT_FAILURE);
		}
		else{ // cpu sigue ejecutando (sale después)
			finalizarCPU = true;
			log_info(logger, "El CPU se cerrará cuando finalice ráfaga actual.");
		}
}

int conectarConUMC(){
	int conexion = conectarSocket(fdUMC, config->ipUMC, config->puertoUMC);
	if(conexion == ERROR){
		log_error(logger, "Falló conexión con UMC.");
		return FALSE;
	}
	else{
		return TRUE;
	}
}

void obtenerTamanioDePagina(){
	int * tamPagina = malloc(INT);
	recibirPorSocket(fdUMC, tamPagina, INT);
	tamanioPagina = *tamPagina; // Seteo el tamaño de página que recibo de UMC
	free(tamPagina); tamPagina = NULL;
	log_info(logger, "Recibí tamanio de página: %d.", tamanioPagina);
}

void conectarConNucleo() {
	fdNucleo = nuevoSocket();
	int ret = conectarSocket(fdNucleo, config->ipNucleo, config->puertoNucleo);
	validar_conexion(ret, 1); // Es terminante por ser cliente

	handshake_cliente(fdNucleo, "P");

	int head;
	void* entrada = NULL;
	entrada = aplicar_protocolo_recibir(fdNucleo, &head);
	if(head == TAMANIO_STACK){
		tamanioStack = *((int*) entrada); // Seteo el tamaño de stack que recibo de Núcleo
	}
	log_info(logger, "Recibí tamanio de stack: %d.", tamanioStack);
	free(entrada); entrada = NULL;
}

void revisarFinalizarCPU(){
	if (finalizarCPU){
		log_debug(logger, "Cerrando CPU por señal SIGUSR1 recibida durante ejecución.");
		cerrarSocket(fdNucleo);
		cerrarSocket(fdUMC);
		liberarRecursos();

		exit(EXIT_FAILURE);
	}
}

int recibirYvalidarEstadoDelPedidoAUMC(){

	void* entrada = NULL;
	int head, estadoDelPedido;

	entrada = aplicar_protocolo_recibir(fdUMC, &head);

	if(entrada == NULL){
		log_error(logger, "UMC se ha desconectado.");
		exitFailureCPU();
	}

	if(head == RESPUESTA_PEDIDO){

		estadoDelPedido = *((int*) entrada);
		free(entrada); entrada = NULL;

		if(estadoDelPedido == NO_PERMITIDO){ // retorno false por pedido rechazado:

			log_info(logger, "UMC ha rechazado pedido del proceso actual.");
			return FALSE;
		} // retorno true por pedido acpetado:
		else{
			return TRUE;
		}
	} // retorno false por error conexión UMC:
	else{

		log_error(logger, "No se pudo completar pedido a UMC.");
		return FALSE;
	}
}

void exitProceso(){
	cpuOciosa = true;
	liberarPcbActiva();
	revisarFinalizarCPU();
}

void exitPorErrorUMC(){
	log_info(logger, "Abortando programa actual por error con UMC.");
	aplicar_protocolo_enviar(fdNucleo, ABORTO_PROCESO, &(pcbActual->pid));
	exitProceso();
}

char* solicitarProximaInstruccionAUMC(){
	t_intructions *index = pcbActual->indiceCodigo;
	index += pcbActual->pc;
	t_intructions *instruccion = index;
	int comienzo = instruccion->start;
	int longitud = instruccion->offset;
	/*int index = pcbActual->pc;
	int comienzo = pcbActual->indiceCodigo[index].start;
	int longitud = pcbActual->indiceCodigo[index].offset;*/

	// Obtengo la dirección lógica de la instrucción a partir del índice de código:
	solicitudLectura* direccionInstruccion = (solicitudLectura*)malloc(sizeof(solicitudLectura));
	direccionInstruccion->pagina = comienzo / tamanioPagina;
	direccionInstruccion->offset = comienzo % tamanioPagina;
	direccionInstruccion->tamanio = longitud;

	log_info(logger, "Solicitando Instrucción -> Pagina: %d - Offset: %d - Size: %d.",
			direccionInstruccion->pagina, direccionInstruccion->offset, longitud);
	aplicar_protocolo_enviar(fdUMC, PEDIDO_LECTURA_INSTRUCCION, direccionInstruccion);
	free(direccionInstruccion);

	if(recibirYvalidarEstadoDelPedidoAUMC()){
		int head;
		void* entrada = NULL;
		entrada = aplicar_protocolo_recibir(fdUMC, &head);

		if(head == DEVOLVER_INSTRUCCION){
			return (char*) entrada; // UMC aceptó el pedido y me devuelve la instrucción
		}
		else { // retorno null por error en el head
			return NULL;
		}
	} // retorno null porque UMC rechazó el pedido
	return NULL;
}

void limpiarInstruccion(char * instruccion){

	char *aux = instruccion;
	int j = 0;
	while (*instruccion != '\0') {
		if (*instruccion != '\t' && *instruccion != '\n' && !iscntrl(*instruccion)) {
			if (j == 0 && isdigit((int )*instruccion)) {
				++instruccion;
			} else {
				*aux++ = *instruccion++;
				j++;
			}
		} else {
			++instruccion;
		}
	}
	*aux = '\0';
}

void liberarRecursos(){
	free(config->ipNucleo); config->ipNucleo = NULL;
	free(config->ipUMC); config->ipUMC = NULL;
	free(config); config = NULL;
	log_destroy(logger); logger = NULL;
	if(pcbActual != NULL) liberarPcbActiva();
}

void exitFailureCPU(){
	log_info(logger, "CPU ha salido del sistema.");
	liberarRecursos(); // Libero memoria utilizada
	cerrarSocket(fdUMC);
	cerrarSocket(fdNucleo);
	exit(EXIT_FAILURE);
}

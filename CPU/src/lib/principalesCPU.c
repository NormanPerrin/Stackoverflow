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
	config = (t_configuracion*)reservarMemoria(sizeof(t_configuracion));
	config->ipNucleo = strdup(config_get_string_value(archivoConfig, "IP_NUCLEO"));
	config->ipUMC = strdup(config_get_string_value(archivoConfig, "IP_UMC"));
	config->puertoNucleo = config_get_int_value(archivoConfig, "PUERTO_NUCLEO");
	config->puertoUMC = config_get_int_value(archivoConfig, "PUERTO_UMC");
}

void liberarPcbActiva(){
	free(pcbActual->indiceCodigo); pcbActual->indiceCodigo = NULL;
	free(pcbActual->indiceEtiquetas); pcbActual->indiceEtiquetas = NULL;
	list_destroy(pcbActual->indiceStack); pcbActual->indiceStack = NULL;
	free(pcbActual); pcbActual = NULL;
}

void atenderSenialSIGUSR1() {
		printf("Se recibió señal SIGUSR1. Notificando a Núcleo.\n");
		int* respuesta = malloc(INT);
		*respuesta = SENIAL;
		aplicar_protocolo_enviar(fdNucleo, SENIAL_SIGUSR1, respuesta);
		free(respuesta);
		if (cpuOciosa){
			printf("Cerrando proceso CPU...\n");
			liberarRecursos();
			exit(EXIT_FAILURE);
		}
		finalizarCPU = true;
		printf("El CPU se cerrará cuando finalice la ráfaga actual.\n");
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
	int * tamPagina = (int*)malloc(INT);
	recibirPorSocket(fdUMC, tamPagina, INT);
	tamanioPagina = *tamPagina; // Seteo el tamaño de página que recibo de UMC
	free(tamPagina);
	printf("Recibí tamanio de página: %d.\n", tamanioPagina);
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
	printf("Recibí tamanio de stack: %d.\n", tamanioStack);
}

void revisarFinalizarCPU(){
	if (finalizarCPU){
		if(cpuOciosa) cerrarSocket(fdNucleo);
		log_debug(logger, "Cerrando CPU por señal SIGUSR1 recibida durante ejecución.");
		cerrarSocket(fdUMC);
		liberarRecursos();
		return;
	}
}

int recibirYvalidarEstadoDelPedidoAUMC(){
// Tras validación: liberarPcbActiva(); revisarFinalizarCPU(); printf("Esperando nuevo proceso...\n");
	int head;
	void* entrada = NULL;
	int* estadoDelPedido = NULL;

	entrada = aplicar_protocolo_recibir(fdUMC, &head);

	if(head == RESPUESTA_PEDIDO){
		estadoDelPedido = (int*)entrada;
	 if(*estadoDelPedido == NO_PERMITIDO){ // retorno false por pedido rechazado
		 printf("UMC ha rechazado un pedido del proceso actual.\n");
		 return FALSE;
		 } // retorno true por pedido acpetado
	 return TRUE;
	} // retorno false por error en el head
	printf("Error durante pedido a UMC.\n");
	return FALSE;
}

void exitProceso(){
	cpuOciosa = true;
	liberarPcbActiva();
	revisarFinalizarCPU();
	printf("Esperando nuevo proceso...\n");
}

void exitPorErrorUMC(){
	log_debug(logger, "Abortando programa actual...");
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

	printf("Solicitando instrucción -> Pagina: %d - Offset: %d - Size: %d.\n",
			direccionInstruccion->pagina, direccionInstruccion->offset, longitud);
	aplicar_protocolo_enviar(fdUMC, PEDIDO_LECTURA_INSTRUCCION, direccionInstruccion);
	free(direccionInstruccion);

	if(recibirYvalidarEstadoDelPedidoAUMC()){
		int head;
		void* entrada = NULL;
		entrada = aplicar_protocolo_recibir(fdUMC, &head);

		if(head == DEVOLVER_INSTRUCCION){
			return (char*)entrada; // UMC aceptó el pedido y me devuelve la instrucción
		}
		else { // retorno null por error en el head
			return NULL;
		}
	} // retorno null porque UMC rechazó el pedido
	return NULL;
}

void limpiarInstruccion(char * instruccion){

	char *p2 = instruccion;
	int a = 0;
	while (*instruccion != '\0') {
		if (*instruccion != '\t' && *instruccion != '\n' && !iscntrl(*instruccion)) {
			if (a == 0 && isdigit((int )*instruccion)) {
				++instruccion;
			} else {
				*p2++ = *instruccion++;
				a++;
			}
		} else {
			++instruccion;
		}
	}
	*p2 = '\0';
}

void liberarRecursos(){
	free(config->ipUMC);
	free(config); config = NULL;
	log_destroy(logger); logger = NULL;
	if(pcbActual != NULL) liberarPcbActiva();
}

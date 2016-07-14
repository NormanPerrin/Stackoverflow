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
	liberarPcb(pcbActual);
}

void atenderSenialSIGUSR1() {
		log_info(logger, "Se recibió señal SIGUSR1. Notificando a Núcleo.");
		aplicar_protocolo_enviar(fdNucleo, SENIAL_SIGUSR1, MSJ_VACIO);
		if (cpuOciosa){
			printf("Cerrando proceso CPU...\n");
			liberarRecursos();
			exit(EXIT_FAILURE);
		}
		finalizarCPU = true;
		log_debug(logger, "El CPU se cerrará cuando finalice la ráfaga actual.");
}

int conectarConUMC(){
	int conexion = conectarSocket(fdUMC, config->ipUMC, config->puertoUMC);
	if(conexion == ERROR){
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
}

void conectarConNucleo() {
	fdNucleo = nuevoSocket();
	int ret = conectarSocket(fdNucleo, config->ipNucleo, config->puertoNucleo);
	validar_conexion(ret, 1); // Es terminante por ser cliente

	handshake_cliente(fdNucleo, "P");

	int head;
	void* entrada = aplicar_protocolo_recibir(fdNucleo, &head);
	if(head == TAMANIO_STACK){
		tamanioStack = *((int*) entrada); // Seteo el tamaño de stack que recibo de Núcleo
	}
	free(entrada);
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
	 /*	Pasos a seguir tras validación:
	  * liberarPcbActiva();
		revisarFinalizarCPU();
		printf("Esperando nuevo proceso...\n");	*/
	int head;
	void* entrada = NULL;
	int* estadoDelPedido = NULL;

	entrada = aplicar_protocolo_recibir(fdUMC, &head);

	if(head == RESPUESTA_PEDIDO){
		estadoDelPedido = (int*)entrada;
		free(entrada);
	 if(*estadoDelPedido == NO_PERMITIDO){ // retorno false por pedido rechazado
		 printf("UMC ha enviado una respuesta de rechazo.\n");
		 free(estadoDelPedido);
		 return FALSE;
		 } // retorno true por pedido acpetado
	 free(estadoDelPedido);
	 return TRUE;
	} // retorno false por error en el head
	free(entrada);
	return FALSE;
}

void exitProceso(){
	cpuOciosa = true;
	liberarPcbActiva();
	revisarFinalizarCPU();
	printf("Esperando nuevo proceso...\n");
}

void exitPorErrorUMC(){
	log_info(logger, "UMC ha rechazado pedido de lectura/escritura durante la ejecución. Abortando programa...");
	aplicar_protocolo_enviar(fdNucleo, ABORTO_PROCESO, &(pcbActual->pid));
	exitProceso();
}

char* solicitarProximaInstruccionAUMC(){
	int index = pcbActual->pc;
	t_intructions instruccionActual = pcbActual->indiceCodigo[index];

		// Obtengo la dirección lógica de la instrucción a partir del índice de código:
		int num_pagina = instruccionActual.start / tamanioPagina;
		int offset = instruccionActual.start - (tamanioPagina*num_pagina);

		solicitudLectura* direccionInstruccion = (solicitudLectura*)malloc(sizeof(solicitudLectura));
		direccionInstruccion->pagina = num_pagina;
		direccionInstruccion->offset = offset;
		direccionInstruccion->tamanio = instruccionActual.offset;

		log_info(logger, "Solicitando a UMC-> Pagina: %d - Offset: %d - Size: %d",
				num_pagina, offset,instruccionActual.offset);
		aplicar_protocolo_enviar(fdUMC, PEDIDO_LECTURA_INSTRUCCION, direccionInstruccion);
		free(direccionInstruccion);

		if(recibirYvalidarEstadoDelPedidoAUMC()){
			int head;
			char * instruccion = NULL;
			void* entrada = aplicar_protocolo_recibir(fdUMC, &head);

			if(head == DEVOLVER_INSTRUCCION){
				instruccion = strdup((char*)entrada);
				free(entrada);
				return instruccion; // UMC aceptó el pedido y me devuelve la instrucción
			}
			else { // retorno null por error en el head
				free(entrada);
				return NULL;
			}
		} // retorno null porque UMC rechazó el pedido
	return NULL;
}

void limpiarInstruccion(char * instruccion){
	char *p2 = instruccion;
	int a = 0;
	while (*instruccion != '\0') {
		if (*instruccion
				!= '\t'&& *instruccion != '\n' && !iscntrl(*instruccion)) {
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
	free(config);
	log_destroy(logger); logger = NULL;
	if(pcbActual != NULL) liberarPcbActiva();
}

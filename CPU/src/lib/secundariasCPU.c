#include "secundariasCPU.h"

// Funciones
void setearValores_config(t_config * archivoConfig){
	config = (t_configuracion*)reservarMemoria(sizeof(t_configuracion));
	config->ipNucleo = strdup(config_get_string_value(archivoConfig, "IP_NUCLEO"));
	config->ipUMC = strdup(config_get_string_value(archivoConfig, "IP_UMC"));
	config->puertoNucleo = config_get_int_value(archivoConfig, "PUERTO_NUCLEO");
	config->puertoUMC = config_get_int_value(archivoConfig, "PUERTO_UMC");
}

// --LOGGER--
void crearLogger(){
	char * archivoLogCPU = strdup("CPU_LOG.log");
	logger = log_create("CPU_LOG.log", archivoLogCPU, TRUE, LOG_LEVEL_INFO);
	free(archivoLogCPU);
	archivoLogCPU = NULL;
}

void ejecutarProcesoActivo(){
	int * pid = malloc(INT);
	*pid = pcbActual->pid;
	aplicar_protocolo_enviar(fdUMC, INDICAR_PID, pid);
	free(pid);
	printf("El Proceso #%d entró en ejecución.\n", pcbActual->pid);

	int quantumActual = infoQuantum->quantum;

		while(quantumActual > 0){
			int pc = pcbActual->pc;
			t_intructions instruccionActual = pcbActual->indiceCodigo[pc]; // Obtengo la próxima instrucción a ejecutar
			ejecutarInstruccion(instruccionActual);
			usleep(infoQuantum->retardoQuantum * 1000);
			quantumActual--;
		}
		if(quantumActual == 0){
			aplicar_protocolo_enviar(fdNucleo, PCB, pcbActual);
			printf("El Proceso #%d finalizó ráfaga de ejecución.\n", pcbActual->pid);
			printf("Esperando nuevo proceso.\n");
				liberarPcbActiva();
		}
}

void ejecutarInstruccion(t_intructions instruccionActual){
	solicitudLectura* direccionInstruccion = (solicitudLectura*)malloc(sizeof(solicitudLectura));
	void* entrada = NULL;
	int head;

	// Obtengo la dirección lógica de la instrucción a partir del índice de código:
	int num_pagina = instruccionActual.start / tamanioPagina;
	int offset = instruccionActual.start - (tamanioPagina*num_pagina);

	direccionInstruccion->pagina = num_pagina;
	direccionInstruccion->offset = offset;
	direccionInstruccion->tamanio = instruccionActual.offset;

	aplicar_protocolo_enviar(fdUMC, PEDIDO_LECTURA_INSTRUCCION, direccionInstruccion);
	free(direccionInstruccion);

	recibirYvalidarEstadoDelPedidoAUMC();

	char * instruccion = NULL;
	entrada = aplicar_protocolo_recibir(fdUMC, &head);
	if(head == DEVOLVER_INSTRUCCION){
		instruccion = strdup( (char*)entrada );
		free(entrada);
	}

	analizadorLinea((char * const)instruccion, &funcionesAnSISOP, &funcionesKernel);
	(pcbActual->pc)++; // Incremento Program Counter del PCB

	free(instruccion);
}

void recibirYvalidarEstadoDelPedidoAUMC(){

	int head;
	void* entrada = NULL;
	int* estadoDelPedido = NULL;

	entrada = aplicar_protocolo_recibir(fdUMC, &head);

	if(head == RESPUESTA_PEDIDO){
			estadoDelPedido = (int*)entrada;
				free(entrada);

	 if(*estadoDelPedido == NO_PERMITIDO){
			printf("UMC ha rechazado un pedido de lectura/escritura del proceso #%d\n", pcbActual->pid);
				free(estadoDelPedido);

			int* _pid = malloc(INT);
			*_pid= pcbActual->pid;

			aplicar_protocolo_enviar(fdNucleo, ABORTO_PROCESO, _pid);
			free(_pid);
			printf("Finalizando ejecución del programa actual.\n");
				liberarPcbActiva();
			printf("Esperando nuevo proceso.\n");
		 }
	}
		free(estadoDelPedido);
}

void liberarPcbActiva(){
	free(pcbActual->indiceCodigo);
	free(pcbActual->indiceEtiquetas);
	free(pcbActual->indiceStack->posicionesArgumentos);
	free(pcbActual->indiceStack->variables);
	free(pcbActual->indiceStack);

	free(pcbActual);
	pcbActual = NULL;
}

int validar_servidor(char *id) {
	if( !strcmp(id, "U") || !strcmp(id, "N") ) {
		printf("Servidor aceptado\n");
		return TRUE;
	} else {
		printf("Servidor rechazado\n");
		return FALSE;
	}
}

int validar_cliente(char *id) {return 0;}

char* charToString(char c) {
	char* caracter = malloc(2);
	*caracter = c;
	*(caracter + 1) = '\0';
	return caracter;
}

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
	int * estadoDelPedido = NULL;
	int protocolo;

	// Obtengo la dirección lógica de la instrucción a partir del índice de código:
	int num_pagina = instruccionActual.start / tamanioPagina;
	int offset = instruccionActual.start - (tamanioPagina*num_pagina);

	direccionInstruccion->pagina = num_pagina;
	direccionInstruccion->offset = offset;
	direccionInstruccion->tamanio = instruccionActual.offset;

	aplicar_protocolo_enviar(fdUMC, PEDIDO_LECTURA, direccionInstruccion);
	free(direccionInstruccion);

	estadoDelPedido = aplicar_protocolo_recibir(fdUMC, RESPUESTA_PEDIDO);

	if(*estadoDelPedido  == NO_PERMITIDO){
		printf("UMC ha rechazado pedido de lectura de instrucción del proceso #%d", pcbActual->pid);
		free(estadoDelPedido );
		abort();
	}

	char * instruccion = NULL;
	instruccion = strdup( (char*)aplicar_protocolo_recibir(fdUMC, DEVOLVER_INSTRUCCION) );

	analizadorLinea((char * const)instruccion, &funcionesAnSISOP, &funcionesKernel);
	(pcbActual->pc)++; // Incremento Program Counter del PCB

	free(estadoDelPedido );
	free(instruccion);
}

void liberarPcbActiva(){
	free(pcbActual->indiceCodigo);
	free(pcbActual->indiceEtiquetas);
	free(pcbActual->indiceStack->listaPosicionesArgumentos);
	free(pcbActual->indiceStack->listaVariablesLocales);
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

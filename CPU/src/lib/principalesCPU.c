#include "principalesCPU.h"

// --LOGGER--
void crearLoggerCPU(){
	char * archivoLogCPU = strdup("CPU_LOG.log");
	logger = log_create("CPU_LOG.log", archivoLogCPU, TRUE, LOG_LEVEL_INFO);
	free(archivoLogCPU);
	archivoLogCPU = NULL;
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

char* solicitarProximaInstruccionAUMC(){
	int pc = pcbActual->pc;
	t_intructions instruccionActual = pcbActual->indiceCodigo[pc];

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

		recibirYvalidarEstadoDelPedidoAUMC(); // validar

		char * instruccion = NULL;
		entrada = aplicar_protocolo_recibir(fdUMC, &head);

		if(head == DEVOLVER_INSTRUCCION){
			instruccion = strdup( (char*)entrada );
			free(entrada);

			return instruccion;
		}
		else {
			return instruccion;
		}
}

void liberarRecursos(){
	free(config->ipUMC);
	free(config);
	log_destroy(logger);
	logger = NULL;
	liberarPcbActiva();
}

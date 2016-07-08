#include "lib/principalesCPU.h"

// Estructuras funcionesAnSISOP
AnSISOP_funciones funcionesAnSISOP = {
		.AnSISOP_definirVariable = definirVariable,
		.AnSISOP_obtenerPosicionVariable = obtenerPosicionVariable,
		.AnSISOP_dereferenciar = dereferenciar,
		.AnSISOP_asignar = asignar,
		.AnSISOP_obtenerValorCompartida	= obtenerValorCompartida,
		.AnSISOP_asignarValorCompartida	= asignarValorCompartida,
		.AnSISOP_irAlLabel	= irAlLabel,
		.AnSISOP_llamarConRetorno = llamarConRetorno,
		.AnSISOP_retornar = retornar,
		.AnSISOP_imprimir = imprimir,
		.AnSISOP_imprimirTexto	= imprimirTexto,
		.AnSISOP_entradaSalida	= entradaSalida,
		.AnSISOP_finalizar = finalizar,
};

AnSISOP_kernel funcionesKernel = {
		.AnSISOP_wait = s_wait,
		.AnSISOP_signal = s_signal,
};

int main(void) {

	leerArchivoDeConfiguracion(RUTA_CONFIG_CPU); // Abro archivo configuración
												// Definida en el file 'general'
	crearLogger();

	conectarConUMC(); // Conexión con UMC

	obtenerTamanioDePagina(); //obtengo tamaño de pagina de UMC

	conectarConNucleo(); // Conexión con Núcleo

	ejecutarProcesos();

	liberarEstructuras(); // Libero memoria reservada para setear config

	return EXIT_SUCCESS;
}

// Funciones CPU:

void ejecutarProcesos(){

	int protocolo;
	void * entrada = aplicar_protocolo_recibir(fdNucleo, &protocolo);

		while (entrada != NULL){
			switch (protocolo){
				case PCB:{
					pcbActual = (pcb*)malloc(sizeof(pcb));
					memcpy(pcbActual, entrada, sizeof(pcb));

					ejecutarProcesoActivo(pcbActual);

				break;
			}
				case QUANTUM_MODIFICADO:{
					info_quantum* nuevoQuantum = (info_quantum*) entrada;
					infoQuantum->quantum = nuevoQuantum->quantum;
					infoQuantum->retardoQuantum = nuevoQuantum->retardoQuantum;
					free(nuevoQuantum);
				break;
			}
				}
					free(entrada);
					entrada = aplicar_protocolo_recibir(fdNucleo, &protocolo);
		}

		cerrarSocket(fdNucleo);
		cerrarSocket(fdUMC);
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

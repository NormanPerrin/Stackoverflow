#include "lib/principalesCPU.h"

// Estructuras funciones AnSISOP:
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
};

AnSISOP_kernel funcionesKernel = {
		.AnSISOP_wait = s_wait,
		.AnSISOP_signal = s_signal,
};

bool finalizarCPU = false;
bool cpuOciosa = true;
bool huboStackOverflow = false;

int main(void) {

	crearLoggerCPU();
	leerArchivoDeConfiguracion(RUTA_CONFIG_CPU);
	fdUMC = nuevoSocket();

	// Manejo de la señal SIGUSR1:
	signal(SIGUSR1, atenderSenialSIGUSR1);

		if (conectarConUMC()){ // Conexión con UMC

			handshake_cliente(fdUMC, "P");
			obtenerTamanioDePagina();

			conectarConNucleo(); // Conexión con Núcleo

			while (TRUE) {
				log_info(logger, "Esperando mensajes de Núcleo.\n"); // Espera activa de mensajes
					if (recibirMensajesDeNucleo() == TRUE) {
					} else {
						return EXIT_SUCCESS;
					}
				}
				return EXIT_SUCCESS;

			} else {
				log_error(logger, "Error en la conexión con UMC.\n");
				return ERROR;
			}
	liberarRecursos(); // Libero memoria reservada
	cerrarSocket(fdUMC);
}

// Funciones CPU:
void atenderSenialSIGUSR1(int value) {
	if (value == SIGUSR1) {
		log_info(logger, "Se recibió señal SIGUSR1. Notificando a Núcleo.");
		/*printf("Cerrando proceso...\n");
		liberarRecursos();
		exit(EXIT_FAILURE);*/
		if (cpuOciosa) {
			cerrarSocket(fdNucleo);
			return;
		}
		finalizarCPU = true;
		aplicar_protocolo_enviar(fdNucleo, SIGUSR, NULL);
		log_debug(logger, "El CPU se cerrará cuando finalice la ráfaga actual.");
	}
}

int recibirMensajesDeNucleo(){

	int head;
	void *mensaje = aplicar_protocolo_recibir(fdNucleo, &head);

	if (mensaje == NULL) {
			log_info(logger, "Se recibió un mensaje NULL de Núcleo. Cerrando conexión.");
			cerrarSocket(fdNucleo);
			return FALSE;
	} else {
		switch (head) {
			case PCB:{
				// Seteo el pcb actual:
				pcbActual = (pcb*)malloc(sizeof(pcb));
				memcpy(pcbActual, mensaje, sizeof(pcb));
				// Le informo a UMC el cambio de proceso activo:
				aplicar_protocolo_enviar(fdUMC, INDICAR_PID, &(pcbActual->pid));
				// Comienzo la ejecución del proceso:
				ejecutarProcesoActivo();

				break;
				}
			case TAMANIO_STACK:{

				tamanioStack = *((int*) mensaje);
				break;
			}
		}
	}
	return TRUE;
}

void ejecutarProcesoActivo(){
	printf("El Proceso #%d entró en ejecución.\n", pcbActual->pid);
	int quantum = pcbActual->quantum;

	while (quantum <= pcbActual->quantum){
		 // Obtengo la próxima instrucción a ejecutar:
		char* proximaInstruccion = solicitarProximaInstruccionAUMC();
		limpiarInstruccion(proximaInstruccion);

		if (proximaInstruccion != NULL) {
			// Llegó una instrucción, analizo si es o no 'end':
			if (pcbActual->pc >= (pcbActual->cantidad_instrucciones - 1) && (strcmp(proximaInstruccion, "end") == 0)){

				// Es 'end'. Finalizo ejecución por EXIT:
				log_info(logger, "El programa actual ha finalizado con éxito.");
				aplicar_protocolo_enviar(fdNucleo, PCB_FIN_EJECUCION, pcbActual);
				liberarPcbActiva();
				revisarFinalizarCPU();
					return;
			}
			// Ejecuto la próxima instrucción:
			analizadorLinea(proximaInstruccion, &funcionesAnSISOP, &funcionesKernel);

			if (huboStackOverflow){

				log_info(logger, "Se ha producido Stack Overflow. Finalizando programa.");
				aplicar_protocolo_enviar(fdNucleo, ABORTO_PROCESO, &(pcbActual->pid));
				printf("Esperando nuevo proceso.\n");
				liberarPcbActiva();
				revisarFinalizarCPU();
					return;
				}

			// Incremento Program Counter del PCB:
			(pcbActual->pc)++;
			usleep(pcbActual->quantum_sleep * 1000);
			quantum--;

		} // fin if not null
		else {
				log_info(logger, "UMC ha rechazado la solicitud de lectura de instrucción. Finalizando programa.");
			aplicar_protocolo_enviar(fdNucleo, ABORTO_PROCESO, &(pcbActual->pid));
				printf("El Proceso #%d finalizó ráfaga de ejecución.\n", pcbActual->pid);
				printf("Esperando nuevo proceso.\n");
			liberarPcbActiva();
			return;
			} // fin else not null
	} // fin while

	// Finalizó ráfaga de ejecución:
	aplicar_protocolo_enviar(fdNucleo, PCB, pcbActual);
		printf("El Proceso #%d finalizó ráfaga de ejecución.\n", pcbActual->pid);
		printf("Esperando nuevo proceso.\n");
	liberarPcbActiva();
	revisarFinalizarCPU(); // TODO
}

void revisarFinalizarCPU() {
	if (finalizarCPU) {
		log_debug(logger, "El CPU está saliendo del sistema.");
		cerrarSocket(fdNucleo);
		cerrarSocket(fdUMC);
		liberarRecursos();
		return;
	}
}

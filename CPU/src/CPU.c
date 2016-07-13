#include "lib/principalesCPU.h"

// Estructuras funciones AnSISOP:
AnSISOP_funciones funcionesAnSISOP = {
		.AnSISOP_definirVariable = definirVariable,
		.AnSISOP_obtenerPosicionVariable = obtenerPosicionVariable,
		.AnSISOP_dereferenciar = dereferenciar,
		.AnSISOP_asignar = asignar,
		.AnSISOP_obtenerValorCompartida	= obtenerValorCompartida,
		.AnSISOP_asignarValorCompartida	= asignarValorCompartida,
		.AnSISOP_irAlLabel = irAlLabel,
		.AnSISOP_llamarConRetorno = llamarConRetorno,
		.AnSISOP_retornar = retornar,
		.AnSISOP_imprimir = imprimir,
		.AnSISOP_imprimirTexto = imprimirTexto,
		.AnSISOP_entradaSalida = entradaSalida };

AnSISOP_kernel funcionesKernel = {
		.AnSISOP_wait = s_wait,
		.AnSISOP_signal = s_signal };

bool finalizarCPU = false;
bool cpuOciosa = true;
bool huboStackOverflow = false;
int devolvioPcb = DEFAULT;

int main(void) {

	crearLoggerCPU();
	leerArchivoDeConfiguracion(RUTA_CONFIG_CPU);
	pcbActual = (pcb*)malloc(sizeof(pcb));
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
int recibirMensajesDeNucleo(){

	int head;
	void *mensaje = aplicar_protocolo_recibir(fdNucleo, &head);
	if (mensaje == NULL) {
			log_info(logger, "El Núcleo se ha desconectado. Cerrando proceso CPU...");
			cerrarSocket(fdNucleo);
			return FALSE;
	} else {
		switch (head) {
			case PCB:{
				int pcb_size = calcularTamanioPCB(mensaje);
				// Seteo el pcb actual que recibo de Núcleo:
				memcpy(pcbActual, (pcb*) mensaje, pcb_size);
				// Le informo a UMC el cambio de proceso activo:
				aplicar_protocolo_enviar(fdUMC, INDICAR_PID, &(pcbActual->pid));
				// Comienzo la ejecución del proceso:
				ejecutarProcesoActivo();

				break;
				}
		} // fin switch head
	} // fin else msj not null
	return TRUE;
}

void ejecutarProcesoActivo(){
	printf("El proceso #%d entró en ejecución.\n", pcbActual->pid);
	int quantum = pcbActual->quantum;

	while (quantum > 0){
		 // Obtengo la próxima instrucción a ejecutar:
		char* proximaInstruccion = solicitarProximaInstruccionAUMC();

		if (proximaInstruccion != NULL){ // Llegó una instrucción, analizo si es o no 'end':
			limpiarInstruccion(proximaInstruccion);
			if (pcbActual->pc >= (pcbActual->cantidad_instrucciones - 1) && (strcmp(proximaInstruccion, "end") == 0)){
				// Es 'end'. Finalizo ejecución por EXIT:
				log_info(logger, "El programa actual ha finalizado con éxito.");
				aplicar_protocolo_enviar(fdNucleo, PCB_FIN_EJECUCION, pcbActual);
				liberarPcbActiva();
				revisarFinalizarCPU();
				printf("Esperando nuevo proceso.\n");
					return;
			} // No es 'end'. Ejecuto la próxima instrucción:

			analizadorLinea(proximaInstruccion, &funcionesAnSISOP, &funcionesKernel);

			if (huboStackOverflow){
				log_info(logger, "Se ha producido Stack Overflow. Finalizando programa...");
				aplicar_protocolo_enviar(fdNucleo, ABORTO_PROCESO, &(pcbActual->pid));
				liberarPcbActiva();
				revisarFinalizarCPU();
				printf("Esperando nuevo proceso...\n");
					return;
				}
			quantum--; // Decremento el quantum actual
			(pcbActual->pc)++; // Incremento Program Counter del PCB

			switch (devolvioPcb) { // TODO: ver este switch
			case POR_IO:
				/*log_debug(ptrLog, "Finalizo ejecucion por operacion I/O");
				finalizarEjecucionPorIO();
				revisarFinalizarCPU();*/
				return;
			case POR_WAIT:
				/*log_debug(ptrLog, "Finalizo ejecucion por un Wait.");
				finalizarEjecucionPorWait();
				revisarFinalizarCPU();*/
				return;
			default:
				break;
			}
			usleep(pcbActual->quantum_sleep * 1000); // Retardo de quantum
		} // fin if not null
		else {
			liberarPcbActiva();
			revisarFinalizarCPU();
			printf("Esperando nuevo proceso...\n");
			return;
			} // fin else not null
	} // fin while que descuenta quantum
	// Finalizó ráfaga de ejecución:
	aplicar_protocolo_enviar(fdNucleo, PCB_FIN_QUANTUM, pcbActual);
	log_debug(logger, "El proceso ha finalizado ráfaga de ejecución.");
	liberarPcbActiva();
	revisarFinalizarCPU();
	printf("Esperando nuevo proceso...\n");
}

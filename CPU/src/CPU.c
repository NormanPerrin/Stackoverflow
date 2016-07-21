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
		.AnSISOP_entradaSalida = entradaSalida};

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
	fdUMC = nuevoSocket();
	// Manejo de la señal SIGUSR1:
	signal(SIGUSR1, atenderSenialSIGUSR1);

		if (conectarConUMC()){ // Conexión con UMC

			handshake_cliente(fdUMC, "P");
			obtenerTamanioDePagina();

			conectarConNucleo(); // Conexión con Núcleo

			while (TRUE){
				printf("Esperando mensajes de Núcleo...\n"); // Espera activa de mensajes
					if (recibirMensajesDeNucleo() == TRUE) {
					} else {
						exitCPU();
						return EXIT_SUCCESS;
					}
				}
				exitCPU();
				return EXIT_SUCCESS;
		}else { // fin else conexión UMC
				log_error(logger, "Error en la conexión con UMC.\n");
				exitCPU();
				return ERROR;
			}
}

void exitCPU(){
	liberarRecursos(); // Libero memoria reservada
	cerrarSocket(fdUMC);
}

// Funciones CPU:
int recibirMensajesDeNucleo(){

	int head;
	void *mensaje = NULL;
	mensaje = aplicar_protocolo_recibir(fdNucleo, &head);
	/*if (mensaje == NULL) {
			log_info(logger, "El Núcleo se ha desconectado. Cerrando proceso CPU...");
			cerrarSocket(fdNucleo);

			return FALSE;
	} else {*/
		if(head == PCB){
				// Seteo la pcb actual que recibo de Núcleo:
				pcbActual = (pcb*) mensaje;
				// Le informo a UMC el cambio de proceso activo:
				int* nuevoPid = malloc(INT);
				*nuevoPid = pcbActual->pid;
				aplicar_protocolo_enviar(fdUMC, INDICAR_PID, nuevoPid);
				free(nuevoPid);
				// Comienzo la ejecución del proceso:
				cpuOciosa = false;
				huboStackOverflow = false;
				devolvioPcb = DEFAULT;

				ejecutarProcesoActivo(); // Ejecuto ráfaga del proceso actual

				return TRUE;
		} // fin else head
		else{
			return FALSE;
		}
	//} // fin else msj not null
}

void ejecutarProcesoActivo(){
	log_info(logger, "El proceso #%d entró en ejecución.\n", pcbActual->pid);
	int quantum = pcbActual->quantum;

	while (quantum > 0){

		 // Obtengo la próxima instrucción a ejecutar:
		char* proximaInstruccion = solicitarProximaInstruccionAUMC();

		if (proximaInstruccion != NULL){ // Llegó una instrucción, analizo si es o no 'end':

			limpiarInstruccion(proximaInstruccion);

			if (pcbActual->pc >= (pcbActual->cantidad_instrucciones -1) && string_starts_with(proximaInstruccion, "end")){
				// Es 'end'. Finalizo ejecución por EXIT:
				log_info(logger, "El programa actual ha finalizado con éxito.");
				aplicar_protocolo_enviar(fdNucleo, PCB_FIN_EJECUCION, pcbActual);
				exitProceso();
					return;
			} // No es 'end'. Ejecuto la próxima instrucción:

			analizadorLinea(proximaInstruccion, &funcionesAnSISOP, &funcionesKernel);

			if (huboStackOverflow){
				log_info(logger, "Se ha producido Stack Overflow. Abortando programa...");
				int* pidActual = malloc(INT);
				*pidActual = pcbActual->pid;
				aplicar_protocolo_enviar(fdNucleo, ABORTO_PROCESO, pidActual);
				free(pidActual);
				exitProceso();
					return;
				}

			quantum--; // Decremento el quantum actual
			(pcbActual->pc)++; // Incremento Program Counter del PCB

			switch (devolvioPcb){
			case POR_IO:{
				log_info(logger, "Expulsando proceso por pedido de I/O.");
				int* pidActual = malloc(INT);
				*pidActual = pcbActual->pid;
				aplicar_protocolo_enviar(fdNucleo, PCB_ENTRADA_SALIDA, pidActual);
				free(pidActual);
				exitProceso();
				return;
			}
			case POR_WAIT:{
				log_info(logger, "Expulsando proceso por operación Wait.");
				int* pidActual = malloc(INT);
				*pidActual = pcbActual->pid;
				aplicar_protocolo_enviar(fdNucleo, PCB_WAIT, pidActual);
				free(pidActual);
				exitProceso();
				return;
			}
		} // fin switch devolvió PCB

			usleep(pcbActual->quantum_sleep * 1000); // Retardo de quantum

		} // fin if not null
		else { // Llegó instrucción null por error con UMC:
			exitPorErrorUMC();
			return;
			} // fin else not null
	} // fin while que descuenta quantum
	// Finalizó ráfaga de ejecución:
	aplicar_protocolo_enviar(fdNucleo, PCB_FIN_QUANTUM, pcbActual);
	log_info(logger, "El proceso ha finalizado ráfaga de ejecución.");
	exitProceso();
}

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
		.AnSISOP_entradaSalida = entradaSalida,
		.AnSISOP_llamarSinRetorno = llamarSinRetorno,
		.AnSISOP_finalizar = finalizar };

AnSISOP_kernel funcionesKernel = {
		.AnSISOP_wait = s_wait,
		.AnSISOP_signal = s_signal };

bool finalizarCPU = false;
bool cpuOciosa = true;
bool huboStackOverflow = false;
bool finalizoPrograma = false;
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
				printf("Esperando nuevo proceso a ejecutar...\n"); // Espera activa de mensajes
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
	liberarRecursos(); // Libero memoria utilizada
	cerrarSocket(fdUMC);
	cerrarSocket(fdNucleo);
	printf("CPU ha salido del sistema.\n");
}

// Funciones CPU:
int recibirMensajesDeNucleo(){

	int head;
	void *mensaje = NULL;

	mensaje = aplicar_protocolo_recibir(fdNucleo, &head);

	if (mensaje == NULL){
		log_info(logger, "Núcleo se ha desconectado.");
		return FALSE;
	} else {
		if(head == PCB){
				// Seteo la pcb actual que recibo de Núcleo:
				pcbActual = (pcb*) mensaje;
				// Le informo a UMC el cambio de proceso activo:
				aplicar_protocolo_enviar(fdUMC, INDICAR_PID, &(pcbActual->pid));
				// Comienzo la ejecución del proceso:
				cpuOciosa = false;
				huboStackOverflow = false;
				devolvioPcb = DEFAULT;
				finalizoPrograma = false;

				ejecutarProcesoActivo(); // Ejecuto ráfaga del proceso actual

				return TRUE;
		} // fin else head
		else{
			log_info(logger, "Mensaje inválido de Núcleo.");
			return FALSE;
		} // fin else head pcb
	} // fin else msj null
}

void ejecutarProcesoActivo(){
	log_info(logger, "El proceso #%d entró en ejecución.\n", pcbActual->pid);
	int quantum = pcbActual->quantum;

	while (quantum > 0){
		 // Obtengo la próxima instrucción a ejecutar:
		char* proximaInstruccion = solicitarProximaInstruccionAUMC();

		if (proximaInstruccion != NULL){ // Llegó una instrucción, analizo si es o no 'end':

			limpiarInstruccion(proximaInstruccion);

			if ( pcbActual->pc >= (pcbActual->cantidad_instrucciones -1)
					&& string_starts_with(proximaInstruccion, "end") ){
				// Es 'end'. Finalizo ejecución por EXIT:
				log_info(logger, "El programa actual ha finalizado con éxito.");
				// Mando solamente el pid, porque al Núcleo ya no le sirve el PCB.
				aplicar_protocolo_enviar(fdNucleo, PCB_FIN_EJECUCION, &(pcbActual->pid));
				free(proximaInstruccion); proximaInstruccion = NULL;
				exitProceso();
				return;
			}
			// Si no es 'end'. Ejecuto la próxima instrucción:
			log_info(logger, "Instrucción recibida: %s", proximaInstruccion);

			analizadorLinea(proximaInstruccion, &funcionesAnSISOP, &funcionesKernel);

			if (huboStackOverflow){
				log_info(logger, "Se ha producido Stack Overflow. Abortando programa...");
				// Mando solamente el pid, porque al Núcleo ya no le sirve el PCB.
				aplicar_protocolo_enviar(fdNucleo, ABORTO_PROCESO, &(pcbActual->pid));
				free(proximaInstruccion); proximaInstruccion = NULL;
				exitProceso();
				return;
			}
			if(finalizoPrograma){
				// Finalizo ejecución por EXIT:
				log_info(logger, "El programa actual ha finalizado con éxito.");
				// Mando solamente el pid, porque al Núcleo ya no le sirve el PCB.
				aplicar_protocolo_enviar(fdNucleo, PCB_FIN_EJECUCION, &(pcbActual->pid));
				free(proximaInstruccion); proximaInstruccion = NULL;
				exitProceso();
				return;
			}

			quantum--; // Decremento el quantum actual
			(pcbActual->pc)++; // Incremento Program Counter del PCB

			switch (devolvioPcb){
			case POR_IO:{
				log_info(logger, "Expulsando proceso por pedido de I/O.");
				aplicar_protocolo_enviar(fdNucleo, PCB_ENTRADA_SALIDA, pcbActual);
				free(proximaInstruccion); proximaInstruccion = NULL;
				exitProceso();
				return;
			}
			case POR_WAIT:{
				log_info(logger, "Expulsando proceso por operación Wait bloqueante.");
				aplicar_protocolo_enviar(fdNucleo, PCB_WAIT, pcbActual);
				free(proximaInstruccion); proximaInstruccion = NULL;
				exitProceso();
				return;
			}
		} // fin switch devolvió PCB

			usleep(pcbActual->quantum_sleep * 1000); // Retardo de quantum

			free(proximaInstruccion); proximaInstruccion = NULL;
		} // fin if not null
		else { // Llegó instrucción null por error o rechazo de UMC:
			exitPorErrorUMC();
			return;
		} // fin else not null
	} // fin while que descuenta quantum
	// Finalizó ráfaga de ejecución:
	aplicar_protocolo_enviar(fdNucleo, PCB_FIN_QUANTUM, pcbActual);
	log_info(logger, "El proceso ha finalizado ráfaga de ejecución.");
	exitProceso();
}

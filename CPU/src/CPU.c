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

		/*if (conectarConUMC()){ // Conexión con UMC

			handshake_cliente(fdUMC, "P");
															// TODO: prueba
//			// variables
//			int *respuesta_solicitud = NULL;
//			int *variable = NULL;
//			char *contenido = NULL;
//			int *head = (int*)reservarMemoria(INT);
//
//			// le indico a UMC el pid de esta CPU
//			int *pid = (int*)reservarMemoria(INT);
//			*pid = 2;
//			aplicar_protocolo_enviar(fdUMC, INDICAR_PID, pid);
//
//			// pido de pid 2 la pagina 0 offset 0 tamanio 5
//			solicitudLectura *prueba_lectura = (solicitudLectura*)reservarMemoria(sizeof(solicitudLectura));
//				prueba_lectura->pagina = 0;
//				prueba_lectura->offset = 0;
//				prueba_lectura->tamanio = 5;
//				aplicar_protocolo_enviar(fdUMC, PEDIDO_LECTURA_INSTRUCCION, prueba_lectura);
//				respuesta_solicitud = (int*)aplicar_protocolo_recibir(fdUMC, head);
//				if(*respuesta_solicitud == PERMITIDO) {
//					printf("<lectura> (pid:2 | pagina:0 | offset:0 | tamanio:5) PERMITIDO\n");
//				} else {
//					printf("<lectura> (pid:2 | pagina:0 | offset:0 | tamanio:5) NO_PERMITIDO\n");
//				}
//				if(*respuesta_solicitud == PERMITIDO) {
//					contenido = (char*)aplicar_protocolo_recibir(fdUMC, head);
//					printf("<lectura> Contenido: %s\n", contenido);
//				}
//
//			solicitudEscritura *prueba_escritura = (solicitudEscritura*)reservarMemoria(sizeof(solicitudEscritura));
//				prueba_escritura->contenido = 1;
//				prueba_escritura->offset = 0;
//				prueba_escritura->pagina = 1;
//				aplicar_protocolo_enviar(fdUMC, PEDIDO_ESCRITURA, prueba_escritura);
//				respuesta_solicitud = (int*)aplicar_protocolo_recibir(fdUMC, head);
//				if(*respuesta_solicitud == PERMITIDO) {
//					printf("<lectura> (pid:2 | pagina:0 | offset:0 | tamanio:5) PERMITIDO\n");
//				} else {
//					printf("<lectura> (pid:2 | pagina:0 | offset:0 | tamanio:5) NO_PERMITIDO\n");
//				}
//
//			solicitudLectura *prueba_lectura_variable = (solicitudLectura*)reservarMemoria(sizeof(solicitudLectura));
//				prueba_lectura_variable->pagina = 1;
//				prueba_lectura_variable->offset = 0;
//				aplicar_protocolo_enviar(fdUMC, PEDIDO_LECTURA_VARIABLE, prueba_lectura_variable);
//				respuesta_solicitud = (int*)aplicar_protocolo_recibir(fdUMC, head);
//				if(*respuesta_solicitud == PERMITIDO) {
//					printf("<lectura> (pid:2 | pagina:1 | offset:0) PERMITIDO\n");
//				} else {
//					printf("<lectura> (pid:2 | pagina:1 | offset:0) NO_PERMITIDO\n");
//				}
//				if(*respuesta_solicitud == PERMITIDO) {
//					variable = (int*)aplicar_protocolo_recibir(fdUMC, head);
//					printf("<lectura> Variable: %d\n", *variable);
//				}

			obtenerTamanioDePagina();*/
			tamanioPagina = 2; // TODO: Borrar
			conectarConNucleo(); // Conexión con Núcleo

			while (TRUE){
				log_info(logger, "Esperando mensajes de Núcleo...\n"); // Espera activa de mensajes
					if (recibirMensajesDeNucleo() == TRUE) {
					} else {
						exitCPU();
						return EXIT_SUCCESS;
					}
				}
				exitCPU();
				return EXIT_SUCCESS;
		/*}else { // fin else conexión UMC
				log_error(logger, "Error en la conexión con UMC.\n");
				exitCPU();
				return ERROR;
			}*/
}

void exitCPU(){
	liberarRecursos(); // Libero memoria reservada
	cerrarSocket(fdUMC);

	return EXIT_SUCCESS;
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
				cpuOciosa = false;
				huboStackOverflow = false;
				devolvioPcb = DEFAULT;
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
			if (pcbActual->pc >= (pcbActual->cantidad_instrucciones -1) && (strcmp(proximaInstruccion, "end") == 0)){
				// Es 'end'. Finalizo ejecución por EXIT:
				log_info(logger, "El programa actual ha finalizado con éxito.");
				aplicar_protocolo_enviar(fdNucleo, PCB_FIN_EJECUCION, pcbActual);
				exitProceso();
					return;
			} // No es 'end'. Ejecuto la próxima instrucción:

			analizadorLinea(proximaInstruccion, &funcionesAnSISOP, &funcionesKernel);

			if (huboStackOverflow){
				log_info(logger, "Se ha producido Stack Overflow. Abortando programa...");
				aplicar_protocolo_enviar(fdNucleo, ABORTO_PROCESO, &(pcbActual->pid));
				exitProceso();
					return;
				}
			quantum--; // Decremento el quantum actual
			(pcbActual->pc)++; // Incremento Program Counter del PCB

			switch (devolvioPcb) {
			case POR_IO:{
				log_debug(logger, "Expulsando proceso por pedido de I/O.");
				aplicar_protocolo_enviar(fdNucleo, PCB_ENTRADA_SALIDA, &(pcbActual->pid));
				exitProceso();
				return;
			}
			case POR_WAIT:{
				log_debug(logger, "Expulsando proceso por operación Wait.");
				aplicar_protocolo_enviar(fdNucleo, PCB_WAIT, &(pcbActual->pid));
				exitProceso();
				return;
			}
			default:
				break;
			} // fin switch devolvió PCB
			usleep(pcbActual->quantum_sleep * 1000); // Retardo de quantum
		} // fin if not null
		else {
			exitPorErrorUMC();
			return;
			} // fin else not null
	} // fin while que descuenta quantum
	// Finalizó ráfaga de ejecución:
	aplicar_protocolo_enviar(fdNucleo, PCB_FIN_QUANTUM, pcbActual);
	log_debug(logger, "El proceso ha finalizado ráfaga de ejecución.");
	exitProceso();
}

#include "principales.h"

/*******************************
 *    FUNCIONES PRINCIPALES    *
 ******************************/
void abrirArchivoDeConfiguracion(char * ruta){
	leerArchivoDeConfiguracion(ruta);
	/*if( seteoCorrecto() ){
			log_info(logger, "El archivo de configuración ha sido leído correctamente");
		}*/
}

void inicializarListasYColas(){
	listaCPU = list_create();
	listaConsolas = list_create();
	listaProcesos = list_create();
	colaReady = queue_create();
	colaBlock = queue_create();
}

void conectarConUMC(){
	fd_clienteUMC = nuevoSocket();
	int ret = conectarSocket(fd_clienteUMC, config->ipUMC, config->puertoUMC);
	validar_conexion(ret, 1); // al ser cliente es terminante
	handshake_cliente(fd_clienteUMC, "N");

	int * tamPagina = (int*)malloc(INT);
	recibirPorSocket(fd_clienteUMC, tamPagina, INT);
	tamanioPagina = *tamPagina; // setea el tamaño de pág. que recibe de UMC

	cerrarSocket(fd_clienteUMC);
}

void escucharAConsola(){
int puerto = config->puertoPrograma;

int newfd, escuchaNucleo, maxfd;

	escuchaNucleo = nuevoSocket();
	asociarSocket(escuchaNucleo, puerto);
	escucharSocket(escuchaNucleo, CONEXIONES_PERMITIDAS);
	fd_set readfd, master;
	FD_ZERO(&readfd);
	FD_ZERO(&master);
	FD_SET(escuchaNucleo, &master);
	maxfd = escuchaNucleo;

	while(TRUE) {

	readfd = master;

	seleccionarSocket(maxfd, &readfd, NULL, NULL, NULL, NULL);

	int i;
	for(i = 0; i <= maxfd; i++) { // recorro los fds buscando si alguno se modificó

	if( FD_ISSET(i, &readfd) ) { // i está modificado. Si no está modificado no se le hace nada

	if(i == escuchaNucleo) { // nueva conexión
		int ret_handshake = 0;

		while(ret_handshake == 0) { // Mientras que no acepte la conexión, por error o inválida
			newfd = aceptarConexionSocket(escuchaNucleo);

	if (validar_conexion(newfd, 0) == FALSE) {
		continue;
	} else {
		ret_handshake = handshake_servidor(newfd, "N");
	}
		} // - conexión válida de handshake -

		FD_SET(newfd, &master);
		if(newfd > maxfd) maxfd = newfd;

	} else { // si no es una nueva conexión entonces es un nuevo mensaje
		int tamanioMensaje;
		consola * unaConsolaActiva = (consola *)list_get(listaConsolas, i);

	int* head = (int*)malloc(INT);
	*head = ENVIAR_SCRIPT;
	void * scriptRecibido = aplicar_protocolo_recibir(i, head, &tamanioMensaje);
	free(head);

	if (scriptRecibido == NULL){ // desconexión o error
	// Fin de Consola -> Fin del programa
		log_info(logger, "Se desconectó la Consola con fd #%d. Finalizando el programa con pid #%d.",
				unaConsolaActiva->fd_consola, unaConsolaActiva->pid);

		finalizarPrograma(unaConsolaActiva->pid);
		cerrarSocket(unaConsolaActiva->fd_consola);
		FD_CLR(unaConsolaActiva->fd_consola, &master);
		break;

	} else { // se leyó correctamente el mensaje
		// Recibo el programa de la Consola
	string * scriptNuevo = (string*)scriptRecibido;
	unaConsolaActiva->programa.tamanio = scriptNuevo->tamanio;
	unaConsolaActiva->programa.cadena = strdup(scriptNuevo->cadena);

	pcb * nuevoPCB = crearPCB(*scriptNuevo);

	if(nuevoPCB!=NULL){
		unaConsolaActiva->pid = nuevoPCB->pid;
		list_add(listaProcesos, nuevoPCB);
		free(nuevoPCB);
	}
	else{
		aplicar_protocolo_enviar(unaConsolaActiva->fd_consola, RECHAZAR_PROGRAMA, NULL, 0);
	// La saco de mi lista de consolas activas:
		cerrarSocket(unaConsolaActiva->fd_consola);
		FD_CLR(unaConsolaActiva->fd_consola, &master);
	}
		free(scriptNuevo);
		break;
	}
	 free(scriptRecibido);
				} // - conexión nueva - else - mensaje nuevo -
			} // - i está modificado -
		} // - recorrido de fds -
	} // - while(true) -
	cerrarSocket(escuchaNucleo);
}

void escucharACPU(){

int puerto = config->puertoCPU;
int newfdCPU, fdEscuchaNucleo, maxfd;
	fdEscuchaNucleo = nuevoSocket();
	asociarSocket(fdEscuchaNucleo, puerto);
	escucharSocket(fdEscuchaNucleo, CONEXIONES_PERMITIDAS);
	fd_set readfds, master;
	FD_ZERO(&readfds);
	FD_ZERO(&master);
	FD_SET(fdEscuchaNucleo, &master);
	maxfd = fdEscuchaNucleo;

	while(TRUE) {

	readfds = master;
	seleccionarSocket(maxfd, &readfds, NULL, NULL, NULL, NULL);
	int i;
	for(i = 0; i <= maxfd; i++) { // recorro los fds buscando si alguno se modificó

		if( FD_ISSET(i, &readfds) ) { // i está modificado. Si no está modificado no se le hace nada

			if(i == fdEscuchaNucleo) { // nueva conexión

			int ret_handshake = 0;

	while(ret_handshake == 0) { // Mientras que no acepte la conexión, por error o inválida
	newfdCPU = aceptarConexionSocket(fdEscuchaNucleo);
		if (validar_conexion(newfdCPU, 0) == FALSE) {
			continue;
		} else {
				ret_handshake = handshake_servidor(newfdCPU, "N");
		}
			} // - conexión válida de handshake -

	FD_SET(newfdCPU, &master);
		if(newfdCPU> maxfd) maxfd = newfdCPU;

		cpu * nuevoCPU = malloc(sizeof(cpu));
		nuevoCPU->fd_cpu = newfdCPU;
		nuevoCPU->disponibilidad = LIBRE;
		list_add(listaCPU, nuevoCPU);
		log_info(logger,"Se ha conectado el CPU con fd: %d", nuevoCPU->fd_cpu);
		planificarProceso();

	} else { // si no es una nueva conexión entonces es un nuevo mensaje
		int head, tamanioMensaje;
		cpu * unCPUActivo = (cpu *)list_get(listaCPU, i);
		void * mensaje = aplicar_protocolo_recibir(unCPUActivo->fd_cpu, &head, &tamanioMensaje);

	 if (mensaje == NULL) { // desconexión o error
		cerrarSocket(unCPUActivo->fd_cpu);
		log_info(logger,"La CPU con fd %d se ha desconectado", unCPUActivo->fd_cpu);
		free(list_remove(listaCPU, unCPUActivo->fd_cpu));
		FD_CLR(unCPUActivo->fd_cpu, &master);
		break;

} else { // se leyó correctamente el mensaje
	switch(head){
		case ENVIAR_PCB:{

		pcb * pcbNuevo=(pcb *) mensaje; // Recibo la PCB actualizada del CPU
		log_info(logger, "Fin de quantum de CPU #%d - Proceso #%d", unCPUActivo->fd_cpu, pcbNuevo->pid);
		actualizarDatosDePcbEjecutada(unCPUActivo, pcbNuevo);
		planificarProceso();
		break;
	}
		case FIN_QUANTUM:{
				// completar
		break;
	}
									            	}
	printf("CPU #%d: %d\n", unCPUActivo->fd_cpu, head);
					}
				} // - conexión nueva - else - mensaje nuevo -
			} // - i está modificado -
		} // - recorrido de fds -
	} // - while(true) -
	cerrarSocket(fdEscuchaNucleo);
}

void liberarTodaLaMemoria(){
	limpiarListasYColas();

	log_destroy(logger);
	logger = NULL;
}

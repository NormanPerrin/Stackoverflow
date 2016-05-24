#include "fnucleo.h"

// -- CONFIGURACIÓN INCIAL --
void abrirArchivoDeConfiguracion(char * ruta){
	leerArchivoDeConfiguracion(ruta);
	/*if( seteoCorrecto() ){
			log_info(logger, "El archivo de configuración ha sido leído correctamente");
		}*/
}

void setearValores_config(t_config * archivoConfig){
	config = (t_configuracion*)reservarMemoria(sizeof(t_configuracion));
	char ** aux_vectorStrings;

	config->puertoPrograma = config_get_int_value(archivoConfig, "PUERTO_PROG");
	config->puertoCPU = config_get_int_value(archivoConfig, "PUERTO_CPU");
	config->puertoUMC = config_get_int_value(archivoConfig, "PUERTO_UMC");
	config->ipUMC = strdup(config_get_string_value(archivoConfig, "IP_UMC"));
	config->quantum = config_get_int_value(archivoConfig, "QUANTUM");
	config->retardoQuantum = config_get_int_value(archivoConfig, "QUANTUM_SLEEP");
	config->semaforosID = config_get_array_value(archivoConfig, "SEM_IDS");
	config->ioID = config_get_array_value(archivoConfig, "IO_IDS");
	config->variablesCompartidas = config_get_array_value(archivoConfig, "SHARED_VARS");
	aux_vectorStrings = config_get_array_value(archivoConfig, "SEM_INIT");
	config->semaforosValInicial = convertirStringsEnNumeros(aux_vectorStrings);
	aux_vectorStrings = config_get_array_value(archivoConfig, "IO_SLEEP");
	config->retardosIO = convertirStringsEnNumeros(aux_vectorStrings);
	config->cantidadPaginasStack = config_get_int_value(archivoConfig, "STACK_SIZE");

	free(aux_vectorStrings);
}

void inicializarListasYColas(){
	listaCPU = list_create();
	listaConsolas = list_create();
	listaProcesos = list_create();
	colaReady = queue_create();
	colaBlock = queue_create();
}

// --CONEXIONES: CONSOLA(S), UMC Y CPU(S)--
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
	int * head = (int*)malloc(INT);

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

					cpu * unCPUActivo = (cpu *)list_get(listaCPU, i);
					void * mensaje = aplicar_protocolo_recibir(unCPUActivo->fd_cpu, *head);

					 if (mensaje == NULL) { // desconexión o error
						cerrarSocket(unCPUActivo->fd_cpu);
					log_info(logger,"La CPU con fd %d se ha desconectado", unCPUActivo->fd_cpu);
					free(list_remove(listaCPU, unCPUActivo->fd_cpu));
						FD_CLR(unCPUActivo->fd_cpu, &master);
						break;

	} else { // se leyó correctamente el mensaje
		switch(*head){
			case ENVIAR_PCB:
			{
			pcb * pcbNuevo=(pcb *) mensaje; // Recibo la PCB actualizada del CPU
			log_info(logger, "Fin de quantum de CPU #%d - Proceso #%d", unCPUActivo->fd_cpu, pcbNuevo->pid);
			actualizarDatosEnPCBProceso(unCPUActivo, pcbNuevo);
			planificarProceso();
			break;
			}
			case FIN_QUANTUM:
			{
				// continuar
			}
									            	}
			printf("CPU #%d: %d\n", unCPUActivo->fd_cpu, *head);
					}
				} // - conexión nueva - else - mensaje nuevo -
			} // - i está modificado -
		} // - recorrido de fds -
	} // - while(true) -
	free(head);
	cerrarSocket(fdEscuchaNucleo);
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
	int * head = (int*)malloc(INT);

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

					consola * unaConsolaActiva = (consola *)list_get(listaConsolas, i);

					void * mensaje = aplicar_protocolo_recibir(i, *head);

					 if (mensaje == NULL){ // desconexión o error

						cerrarSocket(unaConsolaActiva->fd_consola);
						FD_CLR(unaConsolaActiva->fd_consola, &master);
						break;
			} else { // se leyó correctamente el mensaje
				switch(*head){
				case ENVIAR_SCRIPT:
				{
					t_string * scriptNuevo = (t_string*)mensaje; // Recibo la ruta del script de una Consola
					unaConsolaActiva->nombrePrograma = strdup(scriptNuevo->texto);

					pcb * nuevoPCB = crearPCB(unaConsolaActiva->nombrePrograma);

					unaConsolaActiva->pid = nuevoPCB->pid;
					list_add(listaProcesos, nuevoPCB);

					break;
				}
				}
				printf("CPU #%d: %d\n", i, *head);
					}

				} // - conexión nueva - else - mensaje nuevo -
			} // - i está modificado -
		} // - recorrido de fds -
	} // - while(true) -
	free(head);
	cerrarSocket(escuchaNucleo);
}

t_metadata_program* parseoInicialDePrograma(char* codigo){
	t_metadata_program* informacionInicial = (t_metadata_program*)malloc(sizeof(t_metadata_program));
	informacionInicial = metadata_desde_literal(codigo);

	return informacionInicial;
}

char* obtenerScriptDesdeArchivo(char * rutaPrograma){
FILE* archivoOriginal = fopen(rutaPrograma, "r+");
long tamanioScript = fileSize(archivoOriginal); // Me da el numero de bytes, y como 1 char=1 byte:
char* codigo = (char*)calloc(tamanioScript+1, sizeof(char));
char c;

while((c=getc(archivoOriginal))!=EOF){
	*codigo++ = c;
}
*codigo='\0';
fclose(archivoOriginal);

return codigo;
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

// --LOGGER--
void crearLogger(){
	char * archivoLogNucleo = strdup("NUCLEO_LOG.log");
	logger = log_create("NUCLEO_LOG.log", archivoLogNucleo, TRUE, LOG_LEVEL_INFO);
	free(archivoLogNucleo);
	archivoLogNucleo = NULL;
}

long fileSize(FILE* f){
	long actual=ftell(f);
	fseek (f,0,SEEK_END);
	long ultimo=ftell(f);
	fseek(f,actual,SEEK_SET);

return ultimo;
}

// PROCESOS - PCB
pcb * crearPCB(char * rutaPrograma){
	pcb * nuevoPcb = (pcb*)malloc(sizeof(pcb));

	char* codigo = obtenerScriptDesdeArchivo(rutaPrograma);
	long tamanioScript = sizeof(codigo);
	int aux_cant = tamanioScript/tamanioPagina;

	int nuevoPid = asignarPid();
	nuevoPcb->pid = nuevoPid;
	nuevoPcb->paginas_codigo = (aux_cant==0)?aux_cant:aux_cant+1;
	nuevoPcb->estado = READY;
	nuevoPcb->quantum = config->quantum; // TODO: provisorio, ver manejo CPU

	t_metadata_program* infoProg;
	infoProg = parseoInicialDePrograma(codigo);

	nuevoPcb->pc = infoProg->instruccion_inicio; // TODO: o se lo manda = 0

	// Inicializo los tamaños de los índices:
	nuevoPcb->indiceCodigo.tamanio = 2 * INT * infoProg->instrucciones_size;
	nuevoPcb->indiceEtiquetas.tamanio = infoProg->etiquetas_size;
	nuevoPcb->indiceStack.tamanio = config->cantidadPaginasStack * tamanioPagina;


	iniciar_programa_t* nuevoPrograma = (iniciar_programa_t*)malloc(sizeof(iniciar_programa_t));
	nuevoPrograma->paginas = config->cantidadPaginasStack + nuevoPcb->paginas_codigo;
	nuevoPrograma->pid = nuevoPid;
	nuevoPrograma->codigo.texto = strdup(codigo);
	nuevoPrograma->codigo.tamanio = tamanioScript;

	aplicar_protocolo_enviar(fd_clienteUMC, INICIAR_PROGRAMA, nuevoPrograma);

	free(codigo);
	free(infoProg);
	free(nuevoPrograma);

	return nuevoPcb;
}

int asignarPid(){
	int randomPid = rand() % 1000; // número aleatorio entre 0 y 1000

	while ( noSeRepitePid(randomPid) ){
		randomPid = rand() % 1000;
	}
	return randomPid;
}

int noSeRepitePid(int pid){
	int i;
	pcb * unPcb;
	for (i = 0; i < list_size(listaProcesos); i++){
		unPcb = (pcb *)list_get(listaProcesos, i);
		if(unPcb->pid == pid){
			return FALSE;
		}
	}
	return TRUE;
}

void liberarPCB(pcb * pcb){
	free(pcb);
	pcb = NULL;
}

// -- PLANIFICACIÓN --
void ejecutarPrograma(char * nombrePrograma){
	pcb * nuevoPcb = crearPCB(nombrePrograma);
	list_add(listaProcesos, nuevoPcb);

	queue_push(colaReady, nuevoPcb);

	planificarProceso();
}

void planificarProceso(){
		int i, asignado = 0;
		for (i = 0; (i < list_size(listaCPU) && asignado==0); i++){
			cpu * unCPU = (cpu *)list_get(listaCPU, i);
			if (unCPU->disponibilidad==LIBRE){
				if (list_size(colaReady->elements) > 0){
					pcb * unPCB = (pcb *)queue_pop(colaReady);
					unPCB->estado = EXEC;
					unPCB->fdCPU = unCPU->fd_cpu;
					unCPU->disponibilidad = OCUPADO;

					aplicar_protocolo_enviar(unCPU->fd_cpu, ENVIAR_PCB, unPCB);

					asignado = 1;
				}
			}
		}
	}

pcb * buscarProcesoPorPid(int pid){
	int i;
	pcb * unPcb;
	for (i = 0; i < list_size(listaProcesos); i++){
		unPcb = (pcb *)list_get(listaProcesos, i);
		if(unPcb->pid == pid){
			return unPcb; // la pcb del proceso es unPcb
		}
	}
	return NULL; // no se encontró el proceso
}

void finalizarPrograma(int pid){
	pcb * procesoAFinalizar = buscarProcesoPorPid(pid);
	if (procesoAFinalizar!=NULL){
		// continuar
	}
}

void actualizarDatosEnPCBProceso(cpu * unCPU, pcb * pcbNuevo){
	/*unCPU->disponibilidad = LIBRE;
	pcb * unPcb = buscarProcesoPorPid(pcbNuevo->pid);*/
	// continuar
}


// --FUNCIONES AUXILIARES--
int* convertirStringsEnNumeros(char ** variablesConfig){
int i = 0;
int n = NELEMS(variablesConfig) - 1;
int * numeros = (int*)calloc(n, sizeof(int));
	while(variablesConfig[i] != NULL){
		numeros[i] = atoi(variablesConfig[i]);
	 	 i++;
	}
	return numeros;
}

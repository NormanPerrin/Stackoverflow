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

void inicializarListas(){
	listaProcesos = list_create();
	listaProcesosListos = list_create();
	listaProcesosBloqueados = list_create();
	listaCPU = list_create();
}

// --CONEXIONES: CONSOLA(S), UMC Y CPU(S)--

void crear_hilos_conexion() {

	// Reservo memoria para pasarle los argumentos a los hilos y que no haya conflictos
	int *cliente_cpu = (int*)reservarMemoria(INT);
	int *cliente_consola = (int*)reservarMemoria(INT);
	pthread_t hilo_cpu, hilo_consola;

	*cliente_cpu = 1; // CPU
	*cliente_consola = 0; // Consola

	pthread_create(&hilo_cpu, NULL, (void*)escuchar_conexiones, cliente_cpu);
	pthread_create(&hilo_consola, NULL, (void*)escuchar_conexiones, cliente_consola);

	pthread_join(hilo_consola, NULL);
	pthread_join(hilo_cpu, NULL);

	free(cliente_cpu);
	free(cliente_consola);
}

void escuchar_conexiones(void *tipo_cliente) {

	int puerto; // seteo puerto según el tipo
	if( ( *((int*)tipo_cliente) ) == 1 ) { // CPU
		puerto = config->puertoCPU;
	} else { // Consola
		puerto = config->puertoPrograma;
	}

	int newfd, listener, maxfd;

	listener = nuevoSocket();
	asociarSocket(listener, puerto);
	escucharSocket(listener, CONEXIONES_PERMITIDAS);

	fd_set readfd, master;

	FD_ZERO(&readfd);
	FD_ZERO(&master);
	FD_SET(listener, &master);

	maxfd = listener;
	uint8_t *head = (uint8_t*)reservarMemoria(1); // 0 .. 255

	while(TRUE) {

		readfd = master;

		seleccionarSocket(maxfd, &readfd, NULL, NULL, NULL, NULL);

		int i;
		for(i = 0; i <= maxfd; i++) { // recorro los fds buscando si alguno se modificó

			if( FD_ISSET(i, &readfd) ) { // i está modificado. Si no está modificado no se le hace nada

				if(i == listener) { // nueva conexión

					int ret_handshake = 0;

					while(ret_handshake == 0) { // Mientras que no acepte la conexión, por error o inválida
						newfd = aceptarConexionSocket(listener);
						if (validar_conexion(newfd, 0) == FALSE) {
							continue;
						} else {
							ret_handshake = handshake_servidor(newfd, "N");
						}
					} // - conexión válida de handshake -

					FD_SET(newfd, &master);
					if(newfd > maxfd) maxfd = newfd;

				} else { // si no es una nueva conexión entonces es un nuevo mensaje

					int status = recibirPorSocket(i, head, 1);

					if ( validar_recive(status, 0) != 1 ) { // desconexión o error
						free(head);
						close(i);
						FD_CLR(i, &master);
						break;
					} else { // se leyó correctamente el mensaje
						printf("CPU #%d: %d\n", i, *head);
					}

				} // - conexión nueva - else - mensaje nuevo -
			} // - i está modificado -
		} // - recorrido de fds -
	} // - while(true) -

	free(head);
	close(listener);
} // Soy servidor, espero mensajes de algún CPU


void conectarConUMC(){
	fd_serverUMC = nuevoSocket();
	int ret = conectarSocket(fd_serverUMC, config->ipUMC, config->puertoUMC);
	validar_conexion(ret, 1); // al ser cliente es terminante
	handshake_cliente(fd_serverUMC, "N");
} // Soy cliente de la UMC, es  decir, soy el que inicia la conexión con ella

// --LOGGER--
void crearLogger(){
	char * archivoLogNucleo = strdup("NUCLEO_LOG.log");
	logger = log_create("NUCLEO_LOG.log", archivoLogNucleo, TRUE, LOG_LEVEL_INFO);
	free(archivoLogNucleo);
	archivoLogNucleo = NULL;
}

// PROCESOS - PCB
pcb * crearPCB(char * unPrograma){
	pcb * nuevoPcb = malloc(sizeof(pcb));

	nuevoPcb->pid = asignarPid();
	nuevoPcb->pc = 0;
	/*nuevoPcb->cantPaginas =;
	nuevoPcb->indiceCodigo =;
	nuevoPcb->indiceEtiquetas =;
	nuevoPcb->indiceStack =;*/
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

// --FUNCIONES AUXILIARES--
int* convertirStringsEnNumeros(char ** variablesConfig){
int i = 0;
int * aux;
	while(variablesConfig[i] != NULL){
		aux[i] = atoi(variablesConfig[i]);
	 	 i++;
	}
	return aux;
}

int validar_cliente(char *id) {
	if( !strcmp(id, "C") || !strcmp(id, "P") ) {
		printf("Cliente aceptado\n");
		return TRUE;
	} else {
		printf("Cliente rechazado\n");
		return FALSE;
	}
}

int validar_servidor(char *id) {
	if(!strcmp(id, "U")) {
		printf("Servidor aceptado\n");
		return TRUE;
	} else {
		printf("Servidor rechazado\n");
		return FALSE;
	}
}

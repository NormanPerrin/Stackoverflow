#include "fnucleo.h"


void setearValores_config(t_config * archivoConfig){
	puertoPrograma = config_get_int_value(archivoConfig, "PUERTO_PROG");
	puertoCPU = config_get_int_value(archivoConfig, "PUERTO_CPU");
	puertoUMC = config_get_int_value(archivoConfig, "PUERTO_UMC");
	ipUMC = strdup(config_get_string_value(archivoConfig, "IP_UMC"));
	quantum = config_get_int_value(archivoConfig, "QUANTUM");
	retardoQuantum = config_get_int_value(archivoConfig, "QUANTUM_SLEEP");
//	pasarCadenasArray(semaforosID, config_get_array_value(archivoConfig, "SEM_IDS"));
//	pasarCadenasArray(ioID, config_get_array_value(archivoConfig, "IO_IDS"));
//	pasarCadenasArray(variablesCompartidas, config_get_array_value(archivoConfig, "SHARED_VARS"));
//	pasarEnterosArray(semaforosValInicial, config_get_array_value(archivoConfig, "SEM_INIT"));
//	pasarEnterosArray(retardosIO, config_get_array_value(archivoConfig, "IO_SLEEP"));
}


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
		puerto = puertoCPU;
	} else { // Consola
		puerto = puertoPrograma;
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

					uint8_t *head = (uint8_t*)reservarMemoria(1); // 0 .. 255
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

	close(listener);
} // Soy servidor, espero mensajes de algún CPU


void conectarConUMC(){
	fd_serverUMC = nuevoSocket();
	int ret = conectarSocket(fd_serverUMC, ipUMC, puertoUMC);
	validar_conexion(ret, 1); // al ser cliente es terminante
	handshake_cliente(fd_serverUMC, "N");
} // Soy cliente de la UMC, es  decir, soy el que inicia la conexión con ella

void crearLogger(){
	char * archivoLogNucleo = strdup("NUCLEO_LOG.log");
	logger = log_create("NUCLEO_LOG.log",archivoLogNucleo,false,LOG_LEVEL_INFO);
	free(archivoLogNucleo);
	archivoLogNucleo = NULL;
}

void testLecturaArchivoDeConfiguracion(){
	printf("Puerto de Programa: %d\n",puertoPrograma);
	printf("Puerto de CPU: %d\n",puertoCPU);
	printf("Quantum de Round Robin: %d\n",quantum);
	printf("Retardo de Quantum: %d\n",retardoQuantum);
	printf("Semaforos: "); imprimirCadenas(semaforosID);
	printf("Cantidad de Semaforos: "); imprimirNumeros(semaforosValInicial);
	printf("Dispositivos de I/O: "); imprimirCadenas(ioID);
	printf("Retardos de I/O: "); imprimirNumeros(retardosIO);
	printf("Variables compartidas: "); imprimirCadenas(variablesCompartidas);
}

// --Funciones MUY auxiliares

void imprimirCadenas(char ** cadenas){
	int i;
	for(i=0; i<NELEMS(cadenas); i++){
		printf("%s, ", cadenas[i]);
			}
}

void imprimirNumeros(int * numeros){
	int i;
	for(i=0; i<NELEMS(numeros); i++){
			printf("%d, ", numeros[i]);
		}
}

void pasarCadenasArray(char ** cadenas, char ** variablesConfig){
	int i = 0;
	while(variablesConfig[i] != NULL){
		cadenas[i]= strdup(variablesConfig[i]);
		i++;
		}
	}

void pasarEnterosArray(int * numeros, char ** variablesConfig){
int i = 0;
char* stringNum;
	while(variablesConfig[i] != NULL){
	 stringNum = strdup(variablesConfig[i]);
	 numeros[i] = atoi(stringNum);
	 	 i++;
	}
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

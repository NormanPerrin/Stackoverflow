#include "fumc.h"

// Globales
t_configuracion *config;
int flag = FALSE; // exit del programa

void abrirArchivoConfig(char *ruta) {

	t_config *configuracion = config_create(ruta);

	t_configuracion *ret = (t_configuracion*)reservarMemoria(sizeof(t_configuracion));

	ret->backlog = config_get_int_value(configuracion, "BACKLOG");
	ret->puerto = config_get_int_value(configuracion, "PUERTO");
	ret->ip_swap = reservarMemoria(CHAR*16);
	ret->ip_swap = config_get_string_value(configuracion, "IP_SWAP");
	ret->puerto_swap = config_get_int_value(configuracion, "PUERTO_SWAP");
	ret->marcos = config_get_int_value(configuracion, "MARCOS");
	ret->marco_size = config_get_int_value(configuracion, "MARCO_SIZE");
	ret->marco_x_proceso = config_get_int_value(configuracion, "MARCO_X_PROC");
	ret->entradas_tlb = config_get_int_value(configuracion, "ENTRADAS_TLB");
	ret->retardo = config_get_int_value(configuracion, "RETARDO");

	config = ret;
}


void escucharANucleo(){
	int fd_escuchaNucleo, fd_nuevoNucleo;

				fd_escuchaNucleo = nuevoSocket();
				asociarSocket(fd_escuchaNucleo, config->puerto);
				escucharSocket(fd_escuchaNucleo, CONEXIONES_PERMITIDAS);
				fd_nuevoNucleo= aceptarConexionSocket(fd_escuchaNucleo);

				printf("Núcleo conectado. Esperando mensajes\n");
				esperarPaqueteDelCliente(fd_escuchaNucleo, fd_nuevoNucleo);

} // Soy servidor, espero mensajes del Núcleo

void escucharACPU(){
	int fd_escuchaCPU, fd_nuevoCPU;

				fd_escuchaCPU = nuevoSocket();
				asociarSocket(fd_escuchaCPU, config->puerto);
				escucharSocket(fd_escuchaCPU, CONEXIONES_PERMITIDAS);
				fd_nuevoCPU= aceptarConexionSocket(fd_escuchaCPU);

				printf("CPU conectado. Esperando mensajes\n");
				esperarPaqueteDelCliente(fd_escuchaCPU, fd_nuevoCPU);

} // Soy servidor, espero mensajes de algún CPU

void conectarConSwap(){
	int fd_serverSwap;

	fd_serverSwap = nuevoSocket();
			asociarSocket(fd_serverSwap, config->puerto_swap);
			conectarSocket(fd_serverSwap, config->ip_swap, config->puerto_swap);
			// Creo un paquete (string) de size PACKAGESIZE, que le enviaré al Swap
			int enviar = 1;
				char message[PACKAGESIZE];

				printf("Conectado al Swap. Ya se puede enviar mensajes. Escriba 'exit' para salir\n");

				while(enviar){
					fgets(message, PACKAGESIZE, stdin);	// Lee una línea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE
					if (!strcmp(message,"exit\n")) enviar = 0; // Chequeo que no se quiera salir
					if (enviar) enviarPorSocket(fd_serverSwap, message, strlen(message) + 1); // Sólo envío si no quiere salir
				}
				close(fd_serverSwap);
		} // Soy cliente del Swap, es  decir, soy el que inicia la conexión con él

void esperarPaqueteDelCliente(int fd_escucha, int fd_nuevoCliente){

	/*escucharSocket(fd_escucha, CONEXIONES_PERMITIDAS);*/// Ponemos a esuchar de nuevo al socket escucha

			char package[PACKAGESIZE];
				int status = 1;		// Estructura que manjea el status de los recieve.
				// Vamos a ESPERAR que nos manden los paquetes, y los imprimos por pantalla
				while (status != 0){
					status = recibirPorSocket(fd_nuevoCliente, (void*) package, PACKAGESIZE);
					if (status != 0) printf("%s", package);

				}
				close(fd_nuevoCliente);
					close(fd_escucha);
}

// Más adelante para implementar hilos

/*void crearHilos() {

	pthread_t hilo_servidor, hilo_consola;

	pthread_create(&hilo_servidor, NULL, (void*)servidor, NULL);
	pthread_create(&hilo_consola, NULL, (void*)consola, NULL);

	pthread_join(hilo_servidor, NULL);
	pthread_join(hilo_consola, NULL);
}


void servidor() {

	int sockCliente;
	int sockServidor = nuevoSocket();
	asociarSocket(sockServidor, config->puerto);
	escucharSocket(sockServidor, config->backlog);

	if(!flag) {
		sockCliente = aceptarConexionSocket(sockServidor);
	}

	char *head = reservarMemoria(CHAR*2);

	while(!flag) {
		recibirPorSocket(sockCliente, head, CHAR*2);
		int head_val = head[0]-'0';
		printf("head: %d\n", head_val);
	}

	free(head);
	close(sockServidor);
}


void consola() {

	printf("Hola! Ingresá \"#\" para salir\n");
	char *mensaje = reservarMemoria(CHAR*20);

	while(!flag) {

		scanf("%s", mensaje);

		if(!strcmp(mensaje, "#")) {
			printf("Saliendo de UMC\n");
			flag = TRUE;
		} else {
			printf("Consola: %s\n", mensaje);
		}

	}

	free(mensaje);
}

void establecerConexionSwap() {
	int sockClienteSwap = nuevoSocket();
	asociarSocket(sockClienteSwap, config->puerto_swap);
	conectarSocket(sockClienteSwap, config->ip_swap, config->puerto_swap);
}
*/

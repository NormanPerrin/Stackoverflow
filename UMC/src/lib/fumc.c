#include "fumc.h"

// Globales
t_configuracion *config;
int exitFlag = FALSE; // exit del programa
int sockClienteDeSwap; // Se lo va a llamar a necesidad en distintas funciones


void setearValores_config(t_config * archivoConfig) {

	config = (t_configuracion*)reservarMemoria(sizeof(t_configuracion));
	config->ip_swap = (char*)reservarMemoria(CHAR*20);

	config->backlog = config_get_int_value(archivoConfig, "BACKLOG");
	config->puerto = config_get_int_value(archivoConfig, "PUERTO");
	config->ip_swap = strdup(config_get_string_value(archivoConfig, "IP_SWAP"));
	config->puerto_swap = config_get_int_value(archivoConfig, "PUERTO_SWAP");
	config->marcos = config_get_int_value(archivoConfig, "MARCOS");
	config->marco_size = config_get_int_value(archivoConfig, "MARCO_SIZE");
	config->marco_x_proceso = config_get_int_value(archivoConfig, "MARCO_X_PROC");
	config->entradas_tlb = config_get_int_value(archivoConfig, "ENTRADAS_TLB");
	config->retardo = config_get_int_value(archivoConfig, "RETARDO");
}


void conectarConSwap(){
	sockClienteDeSwap = nuevoSocket();
	conectarSocket(sockClienteDeSwap, config->ip_swap, config->puerto_swap);
	handshake_cliente(sockClienteDeSwap, "U");
}


void crearHilos() {
	pthread_t hilo_servidor, hilo_consola;
	pthread_create(&hilo_servidor, NULL, (void*)servidor, NULL);
	pthread_create(&hilo_consola, NULL, (void*)consola, NULL);
	pthread_join(hilo_servidor, NULL);
	pthread_join(hilo_consola, NULL);
}


void servidor() {

	int sockServidor = nuevoSocket();
	asociarSocket(sockServidor, config->puerto);
	escucharSocket(sockServidor, config->backlog);

	while(!exitFlag) {
		int sockCliente = aceptarConexionSocket(sockServidor); // TODO fijarse si abortar programa al error del accept()
		handshake_servidor(sockCliente, "U");
		crearHiloCliente(sockCliente); // TODO hacer validación de cliente antes de crearle un hilo para que no sea cualquier gil
	}

	close(sockServidor);
}


void consola() {

	printf("Hola! Ingresá \"#\" para salir\n");
	char *mensaje = reservarMemoria(CHAR*20);

	while(!exitFlag) {

		scanf("%s", mensaje);

		if(!strcmp(mensaje, "#")) {
			printf("Saliendo de UMC\n");
			exitFlag = TRUE;
		} else {
			printf("Consola: %s\n", mensaje);
		}

	}

	free(mensaje);
}


void crearHiloCliente(int sockCliente) {
	pthread_t hilo_cliente;
	pthread_create(&hilo_cliente, NULL, (void*)cliente, &sockCliente);
	pthread_join(hilo_cliente, NULL);
}

void cliente(void* fdCliente) {

	int sockCliente = *((int*)fdCliente);
	char *buff = (char*)reservarMemoria(PACKAGESIZE);
	int ret = 1;

	while(ret > 0 && !exitFlag) { // en 0 se desconecta y en negativo hubo error
		ret = recibirPorSocket(sockCliente, buff, 2); // TODO PACKAGESIZE
		buff[1] = '\0';
		printf("Cliente #%d: %s\n", sockCliente, buff);
	}

	free(buff);
	close(sockCliente);
}


void liberarEstructuraConfig() {
	free(config->ip_swap);
	free(config);
}


void validarArgumentos(int argc, char **argv) {

	if(argc != 2) {
		printf("Debe ingresar la ruta del archivo de configuración como único parámetro\n");
		exit(1);
	}
}

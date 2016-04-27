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


void crearHilos() {

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

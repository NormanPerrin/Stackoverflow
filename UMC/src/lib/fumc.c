#include "fumc.h"

// Globales
int exitFlag = FALSE; // exit del programa

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
	int ret = conectarSocket(sockClienteDeSwap, config->ip_swap, config->puerto_swap);
	validar_conexion(ret, 1);
	handshake_cliente(sockClienteDeSwap, "U");
}


void crearHilos() {
	pthread_t hilo_servidor, hilo_consola;
	pthread_create(&hilo_servidor, NULL, (void*)servidor, NULL);
	pthread_create(&hilo_consola, NULL, (void*)consola, NULL);
	pthread_join(hilo_consola, NULL);
	pthread_join(hilo_servidor, NULL);
}


void servidor() {

	sockServidor = nuevoSocket();
	asociarSocket(sockServidor, config->puerto);
	escucharSocket(sockServidor, config->backlog);

	while(!exitFlag) {

		int ret_handshake = 0;
		int *sockCliente = (int*)reservarMemoria(INT);

		while(ret_handshake == 0) {

			*sockCliente = aceptarConexionSocket(sockServidor);
			if( validar_conexion(*sockCliente, 0) == FALSE ) {
				continue;
			} else {
				ret_handshake = handshake_servidor(*sockCliente, "U");
			}

		}

		crearHiloCliente(sockCliente);
	}

	close(sockServidor);
}


void consola() {

	printf("Hola! Ingresá \"#\" para salir\n");
	char *mensaje = reservarMemoria(CHAR*20);

	while(!exitFlag) {

		scanf("%[^\n]%*c", mensaje);

		if(!strcmp(mensaje, "#")) {
			printf("Saliendo de UMC\n");
			exitFlag = TRUE;
		} else {
			printf("Consola: %s\n", mensaje);
		}

	}

	free(mensaje);
}


void crearHiloCliente(int *sockCliente) {
	pthread_t hilo_cliente;
	pthread_create(&hilo_cliente, NULL, (void*)cliente, sockCliente);
}

void cliente(void* fdCliente) {

	int sockCliente = *((int*)fdCliente);
	uint8_t *head = (uint8_t*)reservarMemoria(1); // 0 .. 255
	int ret = 1;

	while(ret > 0 && !exitFlag) {

		ret = recibirPorSocket(sockCliente, head, 1);

		if( validar_recive(ret, 0) == FALSE) { // Si se desconecta una CPU o hay error en mensaje no pasa nada. Por eso no terminante
			break;
		} else {
			// Aplicar protocolo
			printf("%d\n", *head);
		}

	}

	free(head);
	close(sockCliente);
}


void liberarEstructura() {
	free(config->ip_swap);
	free(config);
}

void liberarRecusos() {
	// liberar otros recursos
	free(memoria);
	free(tabla_paginas);
	liberarEstructura();
}


int validar_cliente(char *id) {
	if( !strcmp(id, "N") || !strcmp(id, "P") ) {
		printf("Cliente aceptado\n");
		return TRUE;
	} else {
		printf("Cliente rechazado\n");
		return FALSE;
	}
}

int validar_servidor(char *id) {
	if(!strcmp(id, "S")) {
		printf("Servidor aceptado\n");
		return TRUE;
	} else {
		printf("Servidor rechazado\n");
		return FALSE;
	}
}


void iniciarEstructuras() {

	int mp_length = config->marcos * config->marco_size;
	memoria = reservarMemoria(mp_length); // Google Chrome be like

	int tp_lenght = config->marcos * sizeof(tp_t);
	tabla_paginas = reservarMemoria(tp_lenght);

}

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <utilidades/general.h>
#include <utilidades/sockets.h>
#include <commons/config.h>
#include "lib/fumc.h"

void servidor();
//void cliente();
void consola();

t_configuracion *config;
int flag = FALSE;


int main(void) {

	// Abro archivo configuración
	config = abrirArchivoConfig("config.txt");

	// Ejecuto los hilos para conecciones y consola
	pthread_t hilo_servidor, hilo_consola;

	pthread_create(&hilo_servidor, NULL, (void*)servidor, NULL);
	pthread_create(&hilo_consola, NULL, (void*)consola, NULL);

	pthread_join(hilo_servidor, NULL);
	pthread_join(hilo_consola, NULL);

	// Cliente Swap
//	int sockClienteSwap = nuevoSocket();
//	asociarSocket(sockClienteSwap, config->puerto_swap);
//	conectarSocket(sockClienteSwap, config->ip_swap, config->puerto_swap);


	return EXIT_SUCCESS;
}

void servidor() {

	int sockServidor = nuevoSocket();
	asociarSocket(sockServidor, config->puerto);
	escucharSocket(sockServidor, config->backlog);
	int sockCliente = aceptarConexionSocket(sockServidor);

	char *head = reservarMemoria(CHAR*2);

	while(!flag) {
		recibirPorSocket(sockCliente, head, CHAR*2);
		int head_val = head[0]-'0';
		printf("head: %d\n", head_val);
	}

	free(head);
}


void consola() {

	printf("Hola! Ingresá \"#\" para salir\n");
	char *mensaje = reservarMemoria(CHAR*20);

	while(!flag) {
		scanf("%s", mensaje);
		printf("Consola: %s\n", mensaje);
	}

	free(mensaje);
}

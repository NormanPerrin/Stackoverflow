#include <stdio.h>
#include <stdlib.h>
#include <utilidades/general.h>
#include <utilidades/sockets.h>
#include <commons/config.h>
#include "lib/fumc.h"


int main(void) {

	// Abro archivo configuración
	t_configuracion *config = abrirArchivoConfig("config.txt");

	// Servidor
	int sockServidor = nuevoSocket();
	asociarSocket(sockServidor, config->puerto);
	escucharSocket(sockServidor, config->backlog);
	int sockCliente = aceptarConexionSocket(sockServidor); //nucleo o cpu

	// Cliente Swap
	int sockClienteSwap = nuevoSocket();
	asociarSocket(sockClienteSwap, config->puerto_swap);
	conectarSocket(sockClienteSwap, config->ip_swap, config->puerto_swap);

	return EXIT_SUCCESS;
}

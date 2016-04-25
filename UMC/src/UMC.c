#include <stdio.h>
#include <stdlib.h>
#include <utilidades/general.h>
#include <commons/config.h>
#include "lib/fumc.h"


int main(void) {

	// Abro archivo configuración
	t_configuracion *config = abrirArchivoConfig("config.txt");
	printf("%s\n", config->ip_swap);


	// Servidor Núcleo

//	int sockServidorNucleo = nuevoSocket();


	// Servidor CPUs



	// Cliente Swap



	return EXIT_SUCCESS;
}

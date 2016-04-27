#include <stdio.h>
#include <stdlib.h>
#include "lib/fumc.h"

int main(void) {

	// Abro archivo configuración
	abrirArchivoConfig("config.txt");

	// Conectarse a Swap
//	establecerConexionSwap();

	// Creo hilos Servidor y Consola
	crearHilos();


	return EXIT_SUCCESS;
}

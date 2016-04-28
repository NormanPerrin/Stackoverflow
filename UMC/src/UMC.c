#include <stdio.h>
#include <stdlib.h>
#include "lib/fumc.h"

int main(void) {

	leerArchivoDeConfiguracion("config.txt"); // Abro archivo de configuración

//	conectarConSwap(); // Conexión con Swap

	crearHilos(); // Creo hilos Servidor y Consola

	liberarEstructuraConfig();
	return EXIT_SUCCESS;
}

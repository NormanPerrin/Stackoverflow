#include <stdio.h>
#include <stdlib.h>
#include "lib/fumc.h"

int main(int argc, char **argv) {

	validarArgumentos(argc, argv);

	leerArchivoDeConfiguracion(argv[1]); // Abro archivo de configuración

//	conectarConSwap(); // Conexión con Swap

	crearHilos(); // Creo hilos Servidor y Consola

	liberarEstructuraConfig();

	return EXIT_SUCCESS;
}

#include <stdio.h>
#include <stdlib.h>
#include "lib/fumc.h"

int main(void) {

	leerArchivoDeConfiguracion(RUTA_CONFIG_UMC); // Abro archivo de configuración

	conectarConSwap(); // Conexión con Swap

	crearHilos(); // Creo hilos Servidor y Consola

	liberarRecusos();

	return EXIT_SUCCESS;
}

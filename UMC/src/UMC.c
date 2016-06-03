#include <stdio.h>
#include <stdlib.h>
#include "lib/fumc.h"

int main(void) {

	leerArchivoDeConfiguracion(RUTA_CONFIG); // Abro archivo de configuración

	iniciarEstructuras(); // Crea memoria y estructuras de administracións

	conectarConSwap(); // Conexión con Swap

	crearHilos(); // Creo hilos Servidor y Consola

	liberarRecusos();

	return EXIT_SUCCESS;
}

#include <stdio.h>
#include <stdlib.h>
#include "lib/fumc.h"

int main(void) {

	abrirArchivoConfig("config.txt"); // Abro archivo de configuración

	conectarConSwap(); // Conexión con Swap

	escucharANucleo(); // Conexión con Núcleo

	escucharACPU(); // Conexión con CPU

	/*void establecerConexionSwap();

	// Creo hilos Servidor y Consola
	crearHilos();*/

	return EXIT_SUCCESS;
}

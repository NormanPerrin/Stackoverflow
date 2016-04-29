#include <stdio.h>
#include <stdlib.h>
#include <parser/parser.h>
#include <utilidades/general.h>
#include "lib/fnucleo.h"

int main(void) {

	leerArchivoDeConfiguracion("config.txt"); // Abro archivo configuración

	testLecturaArchivoDeConfiguracion(); // Comparo con los valores de ejemplo del archivo

	conectarConUMC(); // Conexión con UMC

	escucharAConsola(); // Conexión con Consolas

	escucharACPU(); // Conexión con CPUs

	return EXIT_SUCCESS;
}

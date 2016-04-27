#include <stdio.h>
#include <stdlib.h>
#include <parser/parser.h>
#include <utilidades/general.h>
#include "lib/fnucleo.h"

int main(void) {

	leerArchivoDeConfiguracion("config.txt"); // Abro archivo configuración

	testLecturaArchivoDeConfiguracion(); // COmparo con los valores de ejemplo del archivo

	void escucharAConsola(); // Conexión con Consolas

	void escucharACPU(); // Conexión con CPUs

	return EXIT_SUCCESS;
}

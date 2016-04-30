#include <stdio.h>
#include <stdlib.h>
#include <utilidades/general.h>
#include "lib/fconsola.h"

int main(void) {

	leerArchivoDeConfiguracion("../configConsola.txt"); // Abro archivo configuración

	testLecturaArchivoDeConfiguracion();

	conectarConNucleo(); // Conexión con Núcleo

	return EXIT_SUCCESS;
}

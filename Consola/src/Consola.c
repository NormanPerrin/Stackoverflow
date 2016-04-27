#include <stdio.h>
#include <stdlib.h>
#include <utilidades/general.h>
#include "lib/fconsola.h"

int main(void) {

	leerArchivoDeConfiguracion("config.txt"); // Abro archivo configuración

	conectarConNucleo(); // Conexión con Núcleo

	return EXIT_SUCCESS;
}

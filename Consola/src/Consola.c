#include <stdio.h>
#include <stdlib.h>
#include <utilidades/general.h>
#include "lib/fconsola.h"

int main(void) {

	leerArchivoDeConfiguracion(RUTA_CONFIG_CONSOLA); // Abro archivo configuración

	testLecturaArchivoDeCOnfiguracion();

	conectarConNucleo(); // Conexión con Núcleo

	return EXIT_SUCCESS;
}

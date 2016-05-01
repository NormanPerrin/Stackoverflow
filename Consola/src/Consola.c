#include <stdlib.h>
#include "lib/fconsola.h"

int main(void) {

	leerArchivoDeConfiguracion(RUTA_CONFIG_CONSOLA); // Abro archivo configuración

	testLecturaArchivoDeConfiguracion();

	conectarConNucleo(); // Conexión con Núcleo

	liberarRecusos(); // Libera memoria asignada

	return EXIT_SUCCESS;
}

#include <stdlib.h>
#include <parser/parser.h>
#include <utilidades/general.h>

#include "lib/principalesCPU.h"

int main(void) {

	leerArchivoDeConfiguracion(RUTA_CONFIG_CPU); // Abro archivo configuración

	conectarConUMC(); // Conexión con UMC

	conectarConNucleo(); // Conexión con Núcleo

	liberarEstructura(); // Libero memoria reservada para setear config

	return EXIT_SUCCESS;
}

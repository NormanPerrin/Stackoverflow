#include <stdlib.h>
#include <parser/parser.h>
#include <utilidades/general.h>
#include "lib/fcpu.h"

int main(void) {

	leerArchivoDeConfiguracion(RUTA_CONFIG_CPU); // Abro archivo configuración

	conectarConNucleo(); // Conexión con Núcleo

	conectarConUMC(); // Conexión con UMC

	esperar_ejecucion();

	liberarEstructura(); // Libero memoria reservada para setear config

	return EXIT_SUCCESS;
}

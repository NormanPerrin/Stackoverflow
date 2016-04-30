#include <stdio.h>
#include <stdlib.h>
#include <utilidades/general.h>
#include "lib/fswap.h"

int main(int argc, char **argv) {

	validarArgumentos(argc, argv);

	leerArchivoDeConfiguracion(argv[1]); // Abro archivo configuración

	escucharUMC(); // Conexión con UMC

	liberarEstructuraConfig();
	return EXIT_SUCCESS;
}

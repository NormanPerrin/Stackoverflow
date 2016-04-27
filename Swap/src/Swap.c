#include <stdio.h>
#include <stdlib.h>
#include <utilidades/general.h>
#include "lib/fswap.h"

int main(void) {

	leerArchivoDeConfiguracion("config.txt"); // Abro archivo configuración

	escucharAUMC(); // Conexión con UMC

	return EXIT_SUCCESS;
}

#include <stdio.h>
#include <stdlib.h>
#include <utilidades/general.h>
#include "lib/fswap.h"

int main(void) {

	leerArchivoDeConfiguracion("configSwap.txt"); // Abro archivo configuración

	escucharUMC(); // Conexión con UMC

	liberarEstructuraConfig();
	return EXIT_SUCCESS;
}

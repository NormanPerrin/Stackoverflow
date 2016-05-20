#include <stdlib.h>
#include "lib/fswap.h"

int main(void) {

	leerArchivoDeConfiguracion(RUTA_CONFIG_SWAP); // Abro archivo configuración

	archivoSwap = inicializar();

	escucharUMC(); // Conexión con UMC

	liberarRecusos(); // Libera memoria asignada

	return EXIT_SUCCESS;
}

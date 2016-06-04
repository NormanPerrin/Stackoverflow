#include <stdio.h>
#include <stdlib.h>
#include <utilidades/general.h>

#include "lib/principales.h"

int main(void) {

	abrirArchivoDeConfiguracion(RUTA_CONFIG_NUCLEO);

	inicializarListasYColas();

	conectarConUMC();

	escucharAConsola();

	escucharACPU();

	liberarTodaLaMemoria();

	return EXIT_SUCCESS;
}

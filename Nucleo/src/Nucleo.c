#include <stdio.h>
#include <stdlib.h>
#include <utilidades/general.h>

#include "lib/principales.h"

int main(void) {

	abrirArchivoDeConfiguracion(RUTA_CONFIG_NUCLEO);

	inicializarListasYColas();

	conectarConUMC();

	esperar_y_PlanificarProgramas();

	liberarTodaLaMemoria();

	return EXIT_SUCCESS;
}

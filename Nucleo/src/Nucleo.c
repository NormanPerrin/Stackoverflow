#include <stdio.h>
#include <stdlib.h>
#include <utilidades/general.h>

#include "lib/principales.h"

int main(void) {

	abrirArchivoDeConfiguracion(RUTA_CONFIG_NUCLEO);

	inicializarColecciones();

	llenarDiccionarioSemaforos();

	llenarDiccionarioVarCompartidas();

	conectarConUMC();

	esperar_y_PlanificarProgramas(); // Select de Consolas y CPUs

	liberarMemoriaUtilizada();

	return EXIT_SUCCESS;
}

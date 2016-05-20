#include <stdio.h>
#include <stdlib.h>
#include <utilidades/general.h>
#include "lib/fnucleo.h"

int main(void) {

	abrirArchivoDeConfiguracion(RUTA_CONFIG_NUCLEO);

	inicializarListas();

	conectarConUMC();

	crearHilosEscucharConsolaYCpu();

	return EXIT_SUCCESS;
}

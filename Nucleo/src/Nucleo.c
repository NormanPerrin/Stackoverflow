#include <stdio.h>
#include <stdlib.h>
#include <utilidades/general.h>
#include "lib/fnucleo.h"

int main(void) {

	abrirArchivoDeConfiguracion(RUTA_CONFIG_NUCLEO);

	inicializarListas();
	inicializarColas();

	conectarConUMC();

	escucharAConsola();
	escucharACPU();

	//crearHilosEscucharConsolaYCpu();

	return EXIT_SUCCESS;
}

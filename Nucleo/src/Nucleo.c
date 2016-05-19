#include <stdio.h>
#include <stdlib.h>
#include <utilidades/general.h>
#include "lib/fnucleo.h"

int main(void) {

	abrirArchivoDeConfiguracion(RUTA_CONFIG_NUCLEO);

	inicializarListas();

	conectarConUMC();

	crear_hilos_conexion();

	return EXIT_SUCCESS;
}

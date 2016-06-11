#include <stdlib.h>
#include <parser/parser.h>
#include <utilidades/general.h>

#include "lib/principalesCPU.h"

int main(void) {

	leerArchivoDeConfiguracion(RUTA_CONFIG_CPU); // Abro archivo configuración
	crearLogger();

	conectarConUMC(); // Conexión con UMC

	obtenerTamanioDePagina(); //obtengo tamaño de pagina de UMC

	conectarConNucleo(); // Conexión con Núcleo

	ejecutarProcesos();

	liberarEstructura(); // Libero memoria reservada para setear config

	return EXIT_SUCCESS;
}

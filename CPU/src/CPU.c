#include "lib/principalesCPU.h"

int main(void) {

	leerArchivoDeConfiguracion(RUTA_CONFIG_CPU); // Abro archivo configuración

	crearLogger();

	conectarConUMC(); // Conexión con UMC

	obtenerTamanioDePagina(); //obtengo tamaño de pagina de UMC

	conectarConNucleo(); // Conexión con Núcleo

	ejecutarProcesos();

	liberarEstructuras(); // Libero memoria reservada para setear config

	return EXIT_SUCCESS;
}

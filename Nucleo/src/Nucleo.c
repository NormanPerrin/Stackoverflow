#include <stdio.h>
#include <stdlib.h>
#include <utilidades/general.h>

#include "lib/principales.h"

int main(void) {

	// Acciones preliminares:
	crearLoggerNucleo();
	leerArchivoDeConfiguracion(RUTA_CONFIG_NUCLEO);
	inicializarColecciones();
	llenarDiccionarioSemaforos();
	llenarDiccionarioVarCompartidas();

	// Hilos de E/S:
	lanzarHilosIO();

	pthread_mutex_init(&mutex_planificarProceso, NULL);

	// Conexiones con los módulos:
	conectarConUMC();
	activarConexionConConsolasYCPUs();

	while(TRUE){
		esperar_y_PlanificarProgramas(); // Select de Consolas y CPUs
	 }

	// Acciones finales:
	pthread_mutex_destroy(&mutex_planificarProceso);

	unirHilosIO();

	liberarMemoriaUtilizada();
	cerrarSocket(fd_UMC);
	cerrarSocket(fdEscuchaConsola);
	cerrarSocket(fdEscuchaCPU);

	return EXIT_SUCCESS;
}

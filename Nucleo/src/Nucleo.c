#include <stdio.h>
#include <stdlib.h>
#include <utilidades/general.h>

#include "lib/principales.h"

int main(void) {

	// Acciones preliminares, configuración y setting:
	crearLoggerNucleo();
	leerArchivoDeConfiguracion(RUTA_CONFIG_NUCLEO);
	inicializarColecciones();
	llenarDiccionarioSemaforos();
	llenarDiccionarioVarCompartidas();

	// Creo hilos para cada dispositivo de E/S:
	lanzarHilosIO();

	pthread_mutex_init(&mutex_planificarProceso, NULL);

	// Conexiones con los módulos:
	conectarConUMC();
	iniciarEscuchaConsolasYCPUs();

	while(TRUE){
		// Select de Consolas y CPUs, planificación activa:
		esperar_y_PlanificarProgramas();
	 }

	// Acciones finales, salida del sistema:
	pthread_mutex_destroy(&mutex_planificarProceso);

	// Cierro hilos de los dispositivos de E/S:
	unirHilosIO();

	liberarRecursosUtilizados();
	cerrarSocket(fd_UMC);
	cerrarSocket(fdEscuchaConsola);
	cerrarSocket(fdEscuchaCPU);

	return EXIT_SUCCESS;
}

#include <stdio.h>
#include <stdlib.h>
#include <utilidades/general.h>

#include "lib/principales.h"

int main(void) {
	// Acciones preliminares:
		inicializarColecciones();

	int return_value = EXIT_SUCCESS;

	if (init_ok()){ // Configuración y setting OK:

		llenarDiccionarioSemaforos();
		llenarDiccionarioVarCompartidas();
		// Creo hilos para cada dispositivo de E/S:
			lanzarHilosIO();

			if (conectarConUMC() && iniciarEscuchaDeConsolasYCPUs()) {
				if (crearThreadPlanificacion()) {

					while(TRUE){
						// Select de Consolas, CPUs e Inotify, planificación activa:
						esperar_y_PlanificarProgramas();
						 }
				} else {
					return_value = EXIT_FAILURE;
				}
	// Cierro hilos de los dispositivos de E/S y el de Planificación:
				pthread_mutex_destroy(&mutex_planificarProceso);
				unirHilosIO();
			} else {
				return_value = EXIT_FAILURE;
			}
		} else {
			log_info(logger, "El Núcleo no pudo inicializarse correctamente.");
			return_value = EXIT_FAILURE;
		}
	// Libero memoria y cierro sockets:
		liberarRecursosUtilizados();
		cerrarSocket(fdEscuchaConsola);
		cerrarSocket(fdEscuchaCPU);
		cerrarSocket(fd_UMC);
		inotify_rm_watch(fd_inotify, watch_descriptor);
		cerrarSocket(fd_inotify);

		return return_value;
}

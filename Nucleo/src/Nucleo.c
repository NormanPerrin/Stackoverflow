#include "lib/principales.h"

bool seDesconectoUMC = false;

int main(void){

	// Acciones preliminares de configuración y setting:
	inicializarColecciones();
	crearLoggerNucleo();
	iniciarEscuchaDeInotify();
	leerConfiguracionNucleo();
	llenarDiccionarioSemaforos();
	llenarDiccionarioVarCompartidas();

	// Creo hilos para cada dispositivo de E/S:
	lanzarHilosIO();
	// Conexión con UMC:
	conexionConUMC();
	// Inicializo hilo de planificación:
	pthread_mutex_init(&mutex_planificarProceso, NULL);

	// Select de Consolas, CPUs e Inotify:
	esperar_y_PlanificarProgramas();

	// Cierro hilos abiertos
	pthread_mutex_destroy(&mutex_planificarProceso);
	unirHilosIO();

	// Libero memoria y cierro sockets:
	liberarRecursosUtilizados();
	cerrarSocket(fdEscuchaConsola);
	cerrarSocket(fdEscuchaCPU);
	cerrarSocket(fd_UMC);
	inotify_rm_watch(fd_inotify, watch_descriptor);
	cerrarSocket(fd_inotify);

	return EXIT_SUCCESS;
}

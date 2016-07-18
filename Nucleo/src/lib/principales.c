#include "principales.h"

void inicializarColecciones(){
	listaCPU = list_create();
	listaCPU_SIGUSR1 = list_create();
	listaConsolas = list_create();
	listaProcesos = list_create(); // Lista de todos los procesos en el sistema
	colaListos = queue_create();
	diccionarioIO = dictionary_create();
	diccionarioSemaforos = dictionary_create();
	diccionarioVarCompartidas = dictionary_create();
}

void crearLoggerNucleo(){
	char * archivoLogNucleo = strdup("NUCLEO_LOG.log");
	logger = log_create("NUCLEO_LOG.log", archivoLogNucleo, true, LOG_LEVEL_INFO);
	free(archivoLogNucleo); archivoLogNucleo = NULL;
}

void leerConfiguracionNucleo(){
	t_config * archivoConfig = NULL;
		if (comprobarQueExistaArchivo(RUTA_CONFIG_NUCLEO) == ERROR){
			printf("Error: Archivo de configuración no encontrado.\n");
		}
		archivoConfig = config_create(RUTA_CONFIG_NUCLEO);

		if(setearValoresDeConfig(archivoConfig)){
			printf("Archivo de configuracion leído correctamente.\n");
		}else{
			printf("Lectura incorrecta del archivo de configuración.\n");
		}
}

void llenarDiccionarioSemaforos(){
	int i = 0;
	while (config->semaforosID[i] != '\0'){
		registrarSemaforo(config->semaforosID[i], atoi(config->semaforosValInicial[i]));
      i++;
    }
}

void llenarDiccionarioVarCompartidas(){
	int i = 0;
	while (config->variablesCompartidas[i] != '\0'){
		registrarVariableCompartida(config->variablesCompartidas[i], 0);
      i++;
    }
}

void lanzarHilosIO(){
  int i = 0;
  while (config->ioID[i] != '\0'){
      hiloIO *hilo = crearHiloIO(i);
      pthread_create(&hilo->hiloID, NULL, &entradaSalidaThread, (void*) &hilo->dataHilo);
      dictionary_put(diccionarioIO, config->ioID[i], hilo);
      printf("Lanzando hilo de IO #%d perteneciente al dispositivo '%s'.\n", i, hilo->dataHilo.nombre);
    i++;
    }
}

int conexionConUMC(){

	fd_UMC = nuevoSocket();
	int conexion = conectarSocket(fd_UMC, config->ipUMC, config->puertoUMC);

	if(conexion == ERROR){
		log_error(logger, "Falló conexión con UMC.");
		seDesconectoUMC = true;

		return FALSE;
	}
	handshake_cliente(fd_UMC, "N"); // es terminante por tratarse de UMC

	int * tamPagina = (int*)malloc(INT);
	int recibido = recibirPorSocket(fd_UMC, tamPagina, INT);
	if(recibido <= 0){
		seDesconectoUMC = true;

		return FALSE;
	}else{
		// Seteo el tamaño de página que recibo de UMC
		tamanioPagina = *tamPagina;
		printf("Recibí tamanio de página: %d.\n", *tamPagina);
		free(tamPagina);

		return TRUE;
	}
}

void esperar_y_PlanificarProgramas(){

	fdEscuchaConsola = nuevoSocket();
	asociarSocket(fdEscuchaConsola, config->puertoPrograma);
	escucharSocket(fdEscuchaConsola, CONEXIONES_PERMITIDAS);

	fdEscuchaCPU = nuevoSocket();
	asociarSocket(fdEscuchaCPU, config->puertoCPU);
	escucharSocket(fdEscuchaCPU, CONEXIONES_PERMITIDAS);

	// Bucle principal:
	while(TRUE){

		if(seDesconectoUMC) break; // salgo del bucle si UMC se ha desconectado

		int max_fd;

		// Borra el conjunto maestro:
	    FD_ZERO(&readfds);
	    // Añadir listeners al conjunto maestro:
	    FD_SET(fdEscuchaConsola, &readfds);
	    FD_SET(fdEscuchaCPU, &readfds);
	    FD_SET(fd_inotify, &readfds);
	    // Obtengo el descriptor de fichero mayor entre los listeners:
	    max_fd = obtenerSocketMaximoInicial();

	    seleccionarSocket(max_fd, &readfds , NULL , NULL , NULL, NULL); // función select

	    if (FD_ISSET(fdEscuchaConsola, &readfds)){

	    		aceptarConexionEntranteDeConsola();

	    } else if(FD_ISSET(fdEscuchaCPU, &readfds)){

	    		aceptarConexionEntranteDeCPU();

	    } else if(FD_ISSET(fd_inotify, &readfds)){

	    		atenderCambiosEnArchivoConfig(&max_fd);

	    }else{ // fin if nueva conexión --> nuevo msj

	    	recorrerListaCPUsYAtenderNuevosMensajes();

	    	} // fin else nuevo mensaje
	} // fin del while
} // fin select

void unirHilosIO(){
	/*int i = 0;
	  while (config->ioID[i] != '\0'){
	      hiloIO*hilo = (hiloIO*)dictionary_get(diccionarioIO, config->ioID[i]);
	      pthread_join(hilo->hiloID, NULL);
	      free(hilo); hilo = NULL;
	   i++;
	}*/
	void cerrarHiloIO(char* id, hiloIO* hilo){
		printf("Cerrando hilo de IO del dispositivo '%s'.\n", hilo->dataHilo.nombre);
		pthread_join(hilo->hiloID, NULL); free(hilo); hilo = NULL; }

	 dictionary_iterator(diccionarioIO, (void*) cerrarHiloIO);
}

void liberarRecursosUtilizados(){
	limpiarColecciones();
	limpiarArchivoConfig();
	log_destroy(logger); logger = NULL;
}

void exitNucleo(){
	log_info(logger, "Finalizando proceso Núcleo...");
	liberarRecursosUtilizados();
	cerrarSocket(fdEscuchaConsola);
	cerrarSocket(fdEscuchaCPU);
	cerrarSocket(fd_UMC);
	inotify_rm_watch(fd_inotify, watch_descriptor);
	cerrarSocket(fd_inotify);
	printf("Núcleo ha salido del sistema.\n");
}

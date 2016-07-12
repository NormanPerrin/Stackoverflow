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

void iniciarEscuchaDeInotify(){
	if (watch_descriptor > 0 && fd_inotify > 0) {
		inotify_rm_watch(fd_inotify, watch_descriptor);
	}
	fd_inotify = inotify_init();
		if (fd_inotify > 0) {
			watch_descriptor = inotify_add_watch(fd_inotify, RUTA_CONFIG_NUCLEO, IN_MODIFY);
		}
}

void leerConfiguracionNucleo(){
	t_config * archivoConfig;
		if (comprobarQueExistaArchivo(RUTA_CONFIG_NUCLEO) == ERROR){
			manejarError("Error: Archivo de configuración no encontrado.\n");
		}
		archivoConfig = config_create(RUTA_CONFIG_NUCLEO);

		if(setearValoresDeConfig(archivoConfig)){
			log_info(logger,"Archivo de configuracion leído correctamente.");
		}else{
			log_info(logger,"Lectura incorrecta del archivo de configuración.");
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
      log_info(logger, "Lanzando hilo de IO #%d perteneciente al dispositivo '%s'", i, hilo->dataHilo.nombre);
    i++;
    }
}

int conexionConUMC(){

	fd_UMC = nuevoSocket();
	int conexion = conectarSocket(fd_UMC, config->ipUMC, config->puertoUMC);

	if(conexion == ERROR){
		return FALSE;
	}
	handshake_cliente(fd_UMC, "N");

	int * tamPagina = (int*)malloc(INT);
	recibirPorSocket(fd_UMC, tamPagina, INT);
	tamanioPagina = *tamPagina; // Seteo el tamaño de página que recibo de UMC
	free(tamPagina);

	return TRUE;
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

		int i, j, max_fd;

		// Borra el conjunto maestro:
	    FD_ZERO(&readfds);
	    // Añadir listeners al conjunto maestro:
	    FD_SET(fdEscuchaConsola, &readfds);
	    FD_SET(fdEscuchaCPU, &readfds);
	    FD_SET(fd_inotify, &readfds);
	    // Obtengo el descriptor de fichero mayor entre los listeners:
	    max_fd = obtenerSocketMaximoInicial();

	    // Reviso si el fd de alguna Consola supera al max_fd actual:
	    for ( j = 0 ; j < list_size(listaConsolas) ; j++){
	    	int fd;
	    	consola * unaConsola = (consola *)list_get(listaConsolas, j);
	        fd = unaConsola->fd_consola;
	        if(fd > 0)
	            FD_SET(fd , &readfds);
	        if(fd > max_fd)
	            max_fd = fd;
	    } // fin for consola

	    // Reviso si el fd de algún CPU supera al max_fd actual:
	    for ( i = 0 ; i < list_size(listaCPU) ; i++){
	    	int fd;
	    	cpu * unCPU = (cpu *)list_get(listaCPU, i);
	    	fd = unCPU->fd_cpu;
	    	if(fd > 0)
	    		FD_SET(fd , &readfds);
	    	if(fd > max_fd)
	    		max_fd = fd;
	     } // fin for cpu

	    seleccionarSocket(max_fd, &readfds , NULL , NULL , NULL, NULL); // función select

	    if (FD_ISSET(fdEscuchaConsola, &readfds)){

	    		aceptarConexionEntranteDeConsola();

	    } else if(FD_ISSET(fdEscuchaCPU, &readfds)){

	    		aceptarConexionEntranteDeCPU();

	    } else if(FD_ISSET(fd_inotify, &readfds)){

	    		atenderCambiosEnArchivoConfig(max_fd);

	    }else{ // fin if nueva conexión

	    	recorrerListaCPUsYAtenderNuevosMensajes();

	    } // fin else nuevo mensaje
	} // fin del while
} // fin select

void unirHilosIO(){
	int i = 0;
	  while (config->ioID[i] != '\0'){
	      hiloIO*hilo = (hiloIO*)dictionary_get(diccionarioIO, config->ioID[i]);
	      log_info(logger, "Cerrando hilo de IO %d que pertenece al dispositivo %s", i, hilo->dataHilo.nombre);
	      pthread_join(hilo->hiloID, NULL);
	      free(hilo); hilo = NULL;
	   i++;
	}
}

void liberarRecursosUtilizados(){
	limpiarColecciones();
	limpiarArchivoConfig();
	log_destroy(logger);
	logger = NULL;
}

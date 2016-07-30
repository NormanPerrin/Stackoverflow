#include "principales.h"

void inicializarColecciones(){
	listaCPU = list_create();
	listaCPU_SIGUSR1 = list_create();
	listaProcesosAbortivos = list_create();
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
			log_error(logger, "Archivo de configuración no encontrado.");
		}
		archivoConfig = config_create(RUTA_CONFIG_NUCLEO);

		if(setearValoresDeConfig(archivoConfig)){
			log_info(logger, "Archivo de configuracion leído correctamente.");
		}else{
			log_error(logger, "Lectura incorrecta del archivo de configuración.");
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
		hiloIO* hilo = crearHiloIO(i);
		pthread_create(&hilo->hiloID, NULL, &entradaSalidaThread, (void*) &hilo->dataHilo);
		dictionary_put(diccionarioIO, config->ioID[i], hilo);
		log_info(logger, "Lanzando hilo de E/S #%d del dispositivo '%s'.", i, hilo->dataHilo.nombre);
		i++;
	}
}

void unirHilosIO(){
	int i = 0;
	while (config->ioID[i] != '\0'){
		hiloIO* hilo = (hiloIO*) dictionary_get(diccionarioIO, config->ioID[i]);

		int status;
		void *res;

		status = pthread_cancel(hilo->hiloID);
		if (status != 0) log_error(logger, "El hilo del dispositivo '%s' no pudo cancelarse.", hilo->dataHilo.nombre);

		status = pthread_join(hilo->hiloID, &res);
		if (status != 0) log_error(logger, "El hilo del dispositivo '%s' no pudo unirse.", hilo->dataHilo.nombre);

		if (res == PTHREAD_CANCELED){
			log_info(logger, "El hilo del dispositivo de E/S '%s' se ha cerrado.", hilo->dataHilo.nombre);
		}
		else{
			log_error(logger, "El hilo del dispositivo de E/s '%s' no pudo cerrarse.", hilo->dataHilo.nombre);
		}

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

	int * tamPagina = malloc(INT);
	int recibido = recibirPorSocket(fd_UMC, tamPagina, INT);
	if(recibido <= 0){
		seDesconectoUMC = true;

		return FALSE;
	}else{
		// Seteo el tamaño de página que recibo de UMC
		tamanioPagina = *tamPagina;
		log_info(logger, "Recibí tamanio de página: %d.", *tamPagina);
		free(tamPagina); tamPagina = NULL;

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

		// Borra el conjunto maestro:
	    FD_ZERO(&readfds);
	    // Añadir listeners al conjunto maestro:
	    FD_SET(fdEscuchaConsola, &readfds);
	    FD_SET(fdEscuchaCPU, &readfds);
	    FD_SET(fd_inotify, &readfds);
	    FD_SET(fd_UMC, &readfds);
	    // Obtengo el descriptor de fichero mayor entre los listeners:
	    max_fd = obtenerSocketMaximoInicial();

	    seleccionarSocket(max_fd, &readfds , NULL , NULL , NULL, NULL); // función select

	    if (FD_ISSET(fdEscuchaConsola, &readfds)){ // nueva conexión consola

	    		aceptarConexionEntranteDeConsola();
	    		planificarProceso();

	    } else if(FD_ISSET(fdEscuchaCPU, &readfds)){ // nueva conexión cpu

	    		aceptarConexionEntranteDeCPU();
	    		planificarProceso();

	    } else if(FD_ISSET(fd_inotify, &readfds)){ // nueva conexión inotify

	    		atenderCambiosEnArchivoConfig();
	    		planificarProceso();

	    }
	    else{ // fin if nueva conexión --> nuevo msj
	    	if(FD_ISSET(fd_UMC, &readfds)){ // nuevo mensaje umc
	    		int head;
	    		void * mensaje = NULL;
	    		mensaje = aplicar_protocolo_recibir(fd_UMC, &head);
	    		if (mensaje == NULL){ // UMC se desconectó, salgo del sistema:
	    			log_info(logger,"UMC se ha desconectado.");
	    		    // Libero memoria y cierro sockets:
	    			pthread_mutex_destroy(&mutex_planificarProceso);
	    			unirHilosIO();
	    		    cerrarSocket(fdEscuchaConsola);
	    		    cerrarSocket(fdEscuchaCPU);
	    		    exitNucleo();

	    		    exit(EXIT_FAILURE);
	    		}
	    	}
	    	planificarProceso();

	    	verificarDesconexionEnConsolas(); // nuevo msj consola
	    	planificarProceso();

	    	recorrerListaCPUsYAtenderNuevosMensajes(); // nuevo msj cpu
	    	planificarProceso();

	    } // fin else nuevo mensaje CPU o desconexión Consola
	} // fin del while
} // fin select

void liberarRecursosUtilizados(){
	limpiarColecciones();
	limpiarArchivoConfig();
	log_destroy(logger); logger = NULL;
}

void exitNucleo(){
	log_info(logger, "Núcleo ha salido del sistema.");
	liberarRecursosUtilizados();
	cerrarSocket(fd_UMC);
	inotify_rm_watch(fd_inotify, watch_descriptor);
	cerrarSocket(fd_inotify);
}

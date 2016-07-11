#include "principales.h"

/*******************************
 *    FUNCIONES PRINCIPALES    *
 ******************************/

void inicializarColecciones(){
	listaCPU = list_create();
	listaConsolas = list_create();
	listaProcesos = list_create(); // Lista de todos los procesos en el sistema
	colaListos = queue_create();
	diccionarioIO = dictionary_create();
	diccionarioSemaforos = dictionary_create();
	diccionarioVarCompartidas = dictionary_create();
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

void llenarDiccionarioSemaforos(){

  int i = 0;
  while (config->semaforosID[i] != '\0'){
      registrarSemaforo(config->semaforosID[i],
          atoi(config->semaforosValInicial[i]));
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
      log_info(logger, "Lanzando hilo de IO %d que pertenece al dispositivo %s", i, hilo->dataHilo.nombre);

      i++;
    }
}

void unirHilosIO(){
	int i = 0;
	  while (config->ioID[i] != '\0'){

	      hiloIO*hilo = (hiloIO*)dictionary_get(diccionarioIO, config->ioID[i]);
	      log_info(logger, "Cerrando hilo de IO %d que pertenece al dispositivo %s", i, hilo->dataHilo.nombre);
	      pthread_join(hilo->hiloID, NULL );
	      free(hilo);
	      hilo = NULL;

	      i++;
	    }
}

int conectarConUMC(){

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

int obtenerSocketMaximoInicial(){
	int max_fd_inicial = 0;

	if (fdEscuchaCPU > fdEscuchaConsola) {
		max_fd_inicial = fdEscuchaCPU;
		} else  {
			max_fd_inicial = fdEscuchaConsola;
		}
	if (max_fd_inicial < fd_inotify) {
		max_fd_inicial = fd_inotify;
	}
	return max_fd_inicial;
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

void recorrerListaCPUsYAtenderNuevosMensajes(){

	int i;
	for ( i = 0; i < list_size(listaCPU); i++){

		cpu * unCPU = (cpu *)list_get(listaCPU, i);
		int fd = unCPU->fd_cpu;

		bool consolaTieneElPidCPU(void* unaConsola){ return (((consola*) unaConsola)->pid) == unCPU->pid;}

		if (FD_ISSET(fd , &readfds)) {

			int protocolo;
		    void * mensaje = aplicar_protocolo_recibir(fd, &protocolo);

		    if (mensaje == NULL){
		    	salvarProcesoEnCPU(unCPU->id);
		    	cerrarSocket(fd);
		    	log_info(logger,"La CPU %i se ha desconectado", unCPU->id);
		    	free(list_remove(listaCPU, i));

		    }else{

	switch(protocolo){

	case PCB_FIN_QUANTUM:{

		pcb * pcbEjecutada = (pcb*) mensaje;
		log_info(logger, "Programa AnSISOP %i fin de quantum en CPU %i.", pcbEjecutada->pid, unCPU->id);
		actualizarPcbEjecutada(unCPU, pcbEjecutada, PCB_FIN_QUANTUM);

		break;
	}
	case PCB_FIN_EJECUCION:{

		pcb * pcbEjecutada = (pcb*) mensaje;
		log_info(logger, "Programa AnSISOP %i fin de ejecución en CPU %i.", pcbEjecutada->pid, unCPU->id);
		actualizarPcbEjecutada(unCPU, pcbEjecutada, PCB_FIN_EJECUCION);

		break;
	}
	case ENTRADA_SALIDA:{

		pedidoIO* datos = (pedidoIO*)mensaje;
		pcb* pcbEjecutada = NULL;
		int head;
		void* entrada = aplicar_protocolo_recibir(fd, &head);
		if(head == PCB_ENTRADA_SALIDA){
			pcbEjecutada = (pcb*)entrada;
		}
		log_info(logger, "Programa AnSISOP %i entrada salida en CPU %i.", pcbEjecutada->pid, unCPU->id);
		// El CPU pasa de Ocupada a Libre:
		unCPU->disponibilidad = LIBRE;
		pcbEjecutada->id_cpu = -1; // El proceso no tiene asignado aún un CPU.
		planificarProceso();
		realizarEntradaSalida(pcbEjecutada, datos);
		liberarPcb(pcbEjecutada);
		free(entrada);
		free(datos);

		break;
	}
	case IMPRIMIR:{

		consola * consolaAsociada = list_find(listaConsolas, (void *)consolaTieneElPidCPU);
		// Le mando el msj a la Consola asociada:
		aplicar_protocolo_enviar(consolaAsociada->fd_consola, IMPRIMIR, string_itoa(*((int*) mensaje)));

		break;
	}
	case IMPRIMIR_TEXTO:{

		consola * consolaAsociada = list_find(listaConsolas, (void *)consolaTieneElPidCPU);
		// Le mando el msj a la Consola asociada:
		aplicar_protocolo_enviar(consolaAsociada->fd_consola, IMPRIMIR_TEXTO, mensaje);

		break;
	}
	case ABORTO_PROCESO:{

		int* pid = (int*)mensaje;
		int index = pcbListIndex(*pid);
		// Le informo a UMC:
		finalizarPrograma(*pid, index);
		bool consolaTieneElPidProceso(void* unaConsola){ return (((consola*) unaConsola)->pid) == *pid;}
		consola * consolaAsociada = list_remove_by_condition(listaConsolas, consolaTieneElPidProceso);
		// Le informo a la Consola asociada:
		aplicar_protocolo_enviar(consolaAsociada->fd_consola, FINALIZAR_PROGRAMA, NULL);
		liberarConsola(consolaAsociada);
		// Quito el PCB del sistema:
		liberarPcb(list_remove(listaProcesos, index));
		free(pid);
		break;
	}
	case SIGNAL_REQUEST:{

		t_semaforo* semaforo = dictionary_get(diccionarioSemaforos, (char*)mensaje);
		semaforo_signal(semaforo);

		break;
	}
	case WAIT_REQUEST:{

		t_semaforo* semaforo = dictionary_get(diccionarioSemaforos, (char*)mensaje);
		if (semaforo_wait(semaforo)){
			// WAIT NO OK: El proceso se bloquea, etonces tomo su pcb.
			aplicar_protocolo_enviar(fd, WAIT_CON_BLOQUEO, NULL);
			pcb* waitPcb = NULL;
			int head;
			void* entrada = aplicar_protocolo_recibir(fd, &head);
			if(head == PCB_WAIT){
				waitPcb = (pcb*)entrada;
			}
			semaforo_blockProcess(semaforo->bloqueados, waitPcb);
			free(waitPcb);
		}
		else{
			// WAIT OK: El proceso no se bloquea, entonces puede seguir ejecutando.
			aplicar_protocolo_enviar(fd, WAIT_SIN_BLOQUEO, NULL);
		}

		break;
	}
	case OBTENER_VAR_COMPARTIDA:{

		// Recibo un char* y devuelvo un int:
		var_compartida* varPedida = dictionary_get(diccionarioVarCompartidas, (char*)mensaje);
		aplicar_protocolo_enviar(fd, DEVOLVER_VAR_COMPARTIDA, &(varPedida->valor));

		break;
	}
	case GRABAR_VAR_COMPARTIDA:{

		var_compartida* var_aGrabar = (var_compartida*)mensaje;
		// Actualizo el valor de la variable solicitada:
		var_compartida* varBuscada = dictionary_get(diccionarioVarCompartidas, var_aGrabar->nombre);
		varBuscada->valor = var_aGrabar->valor;
		free(var_aGrabar->nombre);
		free(var_aGrabar);

		break;
	}
	case SIGUSR: {
		// completar

		break;
	}
	default:
		printf("Recibí el protocolo %i de CPU\n", protocolo);
		break;
	}
	free(mensaje);
		    }
		} // fin if nuevo mensaje fd CPU
	} // fin for listaCPU
}

void aceptarConexionEntranteDeConsola(){

	// Las Conosolas me mandan un único msj con el script ni bien se conectan:
	 int fdNuevaConsola = aceptarConexionSocket(fdEscuchaConsola);

		    int ret_handshake = handshake_servidor(fdNuevaConsola, "N");
		    	if(ret_handshake == FALSE){
		    		perror("[ERROR] Se espera conexión del proceso Consola\n");
		    		cerrarSocket(fdNuevaConsola);
		    	}
		    	else{
	// UNA NUEVA CONSOLA SE HA CONECTADO:
		    		consola * nuevaConsola = malloc(sizeof(consola));

		    		nuevaConsola->id = fdNuevaConsola - fdEscuchaConsola;
		    		nuevaConsola ->fd_consola = fdNuevaConsola;

	// Recibo un nuevo programa desde la Consola:
		    int protocolo;
		    void * entrada = aplicar_protocolo_recibir(nuevaConsola->fd_consola, &protocolo);
		  if(protocolo == ENVIAR_SCRIPT){
			 char * nuevoPrograma = (char*) entrada;
			  free(entrada);
		    nuevaConsola->programa = strdup(nuevoPrograma);

	 // Creo la PCB del programa y pido espacio para los segmentos a UMC:
		    pcb * nuevoPcb = crearPcb(nuevoPrograma);
		    free(nuevoPrograma);

		    if(nuevoPcb == NULL){
	//  UMC no pudo alocar los segmentos del programa, entonces lo rachazo:
		    	aplicar_protocolo_enviar(nuevaConsola->fd_consola, RECHAZAR_PROGRAMA, NULL);
		    	liberarPcb(nuevoPcb);
		    }

		    nuevaConsola->pid = nuevoPcb->pid;

		    list_add(listaConsolas, nuevaConsola );
		    log_info(logger,"La Consola %i se ha conectado", nuevaConsola->id);

	// Pongo al nuevo programa en la cola de Listos:
		    list_add(listaProcesos, nuevoPcb);
		   	queue_push(colaListos, nuevoPcb);

		   planificarProceso();

		  }
		  else{
			  printf("Se espera un script de la Consola #%d.", nuevaConsola->id);
		  }
		}
	}

void aceptarConexionEntranteDeCPU(){

	int fdNuevoCPU = aceptarConexionSocket(fdEscuchaCPU);

		  int ret_handshake = handshake_servidor(fdNuevoCPU, "N");
		  	    if(ret_handshake == FALSE){
		  	    		perror("[ERROR] Se espera conexión del proceso CPU\n");
		  	    		cerrarSocket(fdNuevoCPU);
		  	    	}
		  	    	else{
	// Nueva conexión de CPU:
		    cpu * nuevoCPU = malloc(sizeof(cpu));

		    		nuevoCPU->id = fdNuevoCPU - fdEscuchaCPU;
		    		nuevoCPU->fd_cpu = fdNuevoCPU;
		    		nuevoCPU->disponibilidad = LIBRE;

	// Agrego al CPU a mi lista de CPUs:
		    		list_add(listaCPU, nuevoCPU);
		    		log_info(logger,"La CPU %i se ha conectado", nuevoCPU->id);

	// El nuevo CPU está listo para ejecutar procesos.
		    		planificarProceso();
		    	}
}

void atenderCambiosEnArchivoConfig(int socketMaximo){

	int length, i = 0;
	char buffer[EVENT_BUF_LEN];
	length = read(fd_inotify, buffer, EVENT_BUF_LEN);

	if (length < 0) {
		log_info(logger, "inotify_read: Error al leer el archivo de configuración.");

 } else if (length > 0) {
		struct inotify_event *event = (struct inotify_event *) &buffer[i];
		t_config * new_config = config_create(RUTA_CONFIG_NUCLEO);

			if (new_config && config_has_property(new_config, "QUANTUM")
					&& config_has_property(new_config, "QUANTUM_SLEEP")) {

		config->quantum = config_get_int_value(new_config, "QUANTUM");
		config->retardoQuantum = config_get_int_value(new_config, "QUANTUM_SLEEP");
		log_info(logger, "Se ha modificado el archivo de configuración: Quantum: %d - Quantum Sleep: %d",
				config->quantum, config->retardoQuantum);

		i += EVENT_SIZE + event->len;
		free(new_config);

		FD_CLR(fd_inotify, &readfds);
		iniciarEscuchaDeInotify();

		socketMaximo = (socketMaximo < fd_inotify)? fd_inotify : socketMaximo;
				FD_SET(fd_inotify, &readfds);
			}
		}
}

void liberarRecursosUtilizados(){
	limpiarColecciones();
	limpiarArchivoConfig();
	log_destroy(logger);
	logger = NULL;
}

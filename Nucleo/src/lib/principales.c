#include "principales.h"

/*******************************
 *    FUNCIONES PRINCIPALES    *
 ******************************/
void abrirArchivoDeConfiguracion(char * ruta){
	crearLogger();
	leerArchivoDeConfiguracion(ruta);
	/*if( seteoCorrecto() ){
			log_info(logger, "El archivo de configuración ha sido leído correctamente");
		}*/
}

void inicializarListasYColas(){
	listaCPU = list_create();
	listaConsolas = list_create();
	listaProcesos = list_create(); // listaNuevos
	colaListos = queue_create();
	colaBloqueados = queue_create();
}

void conectarConUMC(){
	fd_UMC = nuevoSocket();
	int ret = conectarSocket(fd_UMC, config->ipUMC, config->puertoUMC);
	validar_conexion(ret, 1); // al ser cliente es terminante
	handshake_cliente(fd_UMC, "N");

	int * tamPagina = (int*)malloc(INT);
	recibirPorSocket(fd_UMC, tamPagina, INT);
	tamanioPagina = *tamPagina; // Seteo el tamaño de pág. que recibo de UMC
	free(tamPagina);
}

void esperar_y_PlanificarProgramas(){
	int i, fd, max_fd;

	fdEscuchaConsola = nuevoSocket();
	asociarSocket(fdEscuchaConsola, config->puertoPrograma);
	escucharSocket(fdEscuchaConsola, CONEXIONES_PERMITIDAS);
	fdEscuchaCPU = nuevoSocket();
	asociarSocket(fdEscuchaCPU, config->puertoCPU);
	escucharSocket(fdEscuchaCPU, CONEXIONES_PERMITIDAS);

	while(TRUE){

	    FD_ZERO(&readfds);
	    FD_SET(fdEscuchaConsola, &readfds);
	    FD_SET(fdEscuchaCPU, &readfds);
	    max_fd = (fdEscuchaConsola > fdEscuchaCPU)?fdEscuchaConsola:fdEscuchaCPU;

	    // Reviso si el fd de alguna Consola supera al max_fd actual:
	    for ( i = 0 ; i < list_size(listaConsolas) ; i++){

	    	consola * unaConsola = (consola *)list_get(listaConsolas, i);
	        fd = unaConsola->fd_consola;
	        if(fd > 0)
	            FD_SET( fd , &readfds);
	        if(fd > max_fd)
	            max_fd = fd;
	    }

	    // Reviso si el fd de algún CPU supera al max_fd actual:
	    for ( i = 0 ; i < list_size(listaCPU) ; i++){

	    	cpu * unCPU = (cpu *)list_get(listaCPU, i);
	    	fd = unCPU->fd_cpu;
	    	if(fd > 0)
	    	FD_SET( fd , &readfds);
	    	if(fd > max_fd)
	    	max_fd = fd;
	     }

	    seleccionarSocket(max_fd, &readfds , NULL , NULL , NULL, NULL);

	    if (FD_ISSET(fdEscuchaConsola, &readfds)) {

	    	aceptarConexionEntranteDeConsola();

	} // fin if nuevaConsola
	    else{

	    	aceptarConexionEntranteDeCPU();

	    } // fin else nuevoCPU
	} // fin while
} // fin función escuchar

void aceptarConexionEntranteDeConsola(){
	int fdNuevaConsola;

	 fdNuevaConsola = aceptarConexionSocket(fdEscuchaConsola);

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
			  string * nuevoPrograma = (string*) entrada;
			  free(entrada);
		    nuevaConsola->programa.cadena = strdup(nuevoPrograma->cadena);
		    nuevaConsola->programa.tamanio = nuevoPrograma->tamanio;
		    	free(nuevoPrograma->cadena);
		    	free(nuevoPrograma);

		    	// Creo la PCB asociada a ese programa:
		    pcb * nuevoPcb = crearPcb(nuevaConsola->programa);
		    if(nuevoPcb == NULL){
		    	//  UMC no pudo alocar los segmentos del programa, lo rachazo:
		    	aplicar_protocolo_enviar(nuevaConsola->fd_consola, RECHAZAR_PROGRAMA, NULL);
		    	liberarPcb(nuevoPcb);
		    }

		    nuevaConsola->pid = nuevoPcb->pid;

		    list_add(listaConsolas, nuevaConsola );
		    log_info(logger,"La Consola %i se ha conectado", nuevaConsola->id);

		    	// Empiezo a correr el nuevo programa:
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
	int fdNuevoCPU, i, fd;

		  fdNuevoCPU = aceptarConexionSocket(fdEscuchaCPU);

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

		    		list_add(listaCPU, nuevoCPU);
		    		log_info(logger,"La CPU %i se ha conectado", nuevoCPU->id);

		    	// Pongo al nuevo CPU a ejecutar un proceso:
		    		planificarProceso();
		    	}

		for ( i = 0 ; i < list_size(listaCPU) ; i++){

		  cpu * unCPU = (cpu *)list_get(listaCPU, i);
		  fd = unCPU->fd_cpu;

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
		   case PCB:{
		          pcb * pcbEjecutada = (pcb *) mensaje;
		          log_info(logger, "Programa AnSISOP %i salió de CPU %i.", pcbEjecutada->pid, unCPU->id);

		          actualizarDatosDePcbEjecutada(unCPU, pcbEjecutada);

		          planificarProceso();

		          break;
		           }
		   case IMPRIMIR:{
			   	string* variableImprimible = (string*)reservarMemoria(sizeof(string));
			   	variableImprimible->cadena = string_itoa(*((int*) mensaje));
			   	variableImprimible->tamanio = string_length(variableImprimible->cadena + 1);

			   bool consolaTieneElPid(void* unaConsola){
			   	return (((consola*) unaConsola)->pid) == unCPU->pid;}
			   consola * consolaAsociada = list_find(listaConsolas, consolaTieneElPid);

			  // Le mando el msj a la Consola asociada:
			 aplicar_protocolo_enviar(consolaAsociada->fd_consola, IMPRIMIR, variableImprimible);
			 free(variableImprimible->cadena);
			 free(variableImprimible);
		   		break;
		   	 }
		   case IMPRIMIR_TEXTO:{
			   bool consolaTieneElPid(void* unaConsola){
			 return (((consola*) unaConsola)->pid) == unCPU->pid;}
			 consola * consolaAsociada = list_find(listaConsolas, consolaTieneElPid);

			 // Le mando el msj a la Consola asociada:
			 aplicar_protocolo_enviar(consolaAsociada->fd_consola, IMPRIMIR_TEXTO, mensaje);
		   		break;
		   		   	 }
		  case FIN_QUANTUM:{
		           // COMPLETAR
		      	  break;

		           default:
		            printf("Recibí el protocolo %i de CPU\n", protocolo);
		            break;
		            	}
		free(mensaje);

		            }
		        }
		    }
		}
}

void liberarTodaLaMemoria(){
	limpiarListasYColas();
	limpiarArchivoConfig();
	log_destroy(logger);
	logger = NULL;
}

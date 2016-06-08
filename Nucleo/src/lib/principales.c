#include "principales.h"

/*******************************
 *    FUNCIONES PRINCIPALES    *
 ******************************/
void abrirArchivoDeConfiguracion(char * ruta){
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

	int * buffer_tamPagina = (int*)malloc(INT);
	recibirPorSocket(fd_UMC, buffer_tamPagina, INT);
	tamanioPagina = *buffer_tamPagina; // setea el tamaño de pág. que recibe de UMC
	free(buffer_tamPagina);

	// cerrarSocket(fd_UMC);
}

void esperar_y_PlanificarProgramas(){
int fdNucleo, fdNuevaConsola, i, fd, max_fd;

	fd_set readfds;
	fdNucleo = newSocket();
	asociarSocket(fdNucleo, config->puertoPrograma);
	escucharSocket(fdNucleo, CONEXIONES_PERMITIDAS);

	while(TRUE){

	    FD_ZERO(&readfds);
	    FD_SET(fdNucleo, &readfds);
	    max_fd = fdNucleo;

	    for ( i = 0 ; i < list_size(listaConsolas) ; i++){

	    	consola * unaConsola = (consola *)list_get(listaConsolas, i);
	        fd = unaConsola->fd_consola;
	        if(fd > 0)
	            FD_SET( fd , &readfds);
	        if(fd > max_fd)
	            max_fd = fd;
	    }

	    seleccionarSocket(max_fd, &readfds , NULL , NULL , NULL, NULL);

	    if (FD_ISSET(fdNucleo, &readfds)) {
	    	fdNuevaConsola = aceptarConexionSocket(fdNucleo);

	    int ret_handshake = handshake_servidor(fdNuevaConsola, "N");
	    	if(ret_handshake == FALSE){
	    		perror("[ERROR] Se espera conexión del proceso Consola\n");
	    		cerrarSocket(fdNuevaConsola);
	    	}
	    	else{
	    	// UNA NUEVA CONSOLA SE HA CONECTADO:
	    		consola * nuevaConsola = malloc(sizeof(consola));

	    		nuevaConsola->id = fdNuevaConsola - fdNucleo;
	    		nuevaConsola ->fd_consola = fdNuevaConsola;

	    	// Recibo un nuevo programa desde la Consola:
	    string * nuevoPrograma = (string *)aplicar_protocolo_recibir(nuevaConsola ->fd_consola, ENVIAR_SCRIPT);
	    nuevaConsola->programa.cadena = strdup(nuevoPrograma->cadena);
	    nuevaConsola->programa->tamanio = nuevoPrograma->tamanio;
	    	free(nuevoPrograma->cadena);
	    	free(nuevoPrograma);

	    	// Creo la PCB asociada a ese programa:
	    pcb * nuevoPcb = crearPcb(nuevaConsola->programa);
	    if(nuevoPcb == NULL){
	    	// la rechazo...
	    	liberarPcb(nuevoPcb);
	    	// ver qué más hacer
	    }

	    nuevaConsola->pid = nuevoPcb->pid;

	    list_add(listaConsolas, nuevaConsola );
	    log_info(logger,"La Consola %i se ha conectado", nuevaConsola->id);

	    	// Empiezo a correr el nuevo programa:
	    list_add(listaProcesos, nuevoPcb);
	   	queue_push(colaListos, nuevoPcb);

	   // planificarProceso();
	    escucharCPUs_y_Planificar();

	    	}
	    }
	  for ( i = 0 ; i < list_size(listaConsolas) ; i++){

	    	consola * unaConsola = (consola *)list_get(listaConsolas, i);
	    	fd = unaConsola->fd_consola;
	   // UNA CONSOLA YA CONECTADA ME MANDA UN MENSAJE:
	    if (FD_ISSET(fd , &readfds)) {

	            int protocolo;
	            void * mensaje = aplicar_protocolo_recibir(fd, &protocolo);
	       if (mensaje == NULL){
	            	cerrarSocket(fd);
	            	log_info(logger,"La Consola %i se ha desconectado", unaConsola->id);
	            	free(list_remove(listaConsolas, i));
	      }else{

	         switch(protocolo){
	         // Ver qué cade de msjs tengo para recibir de Consola
	            		default:
	            			printf("Recibí el protocolo %i de Consola\n", protocolo);
	            			break;
	            	}
	            	free(mensaje);
	            }
	        }
	    }
	}

}

void escucharCPUs_y_Planificar(){
	int fdNucleo, fdNuevoCPU, i, fd, max_fd;

		fd_set readfds;
		fdNucleo = newSocket();
		asociarSocket(fdNucleo, config->puertoCPU);
		escucharSocket(fdNucleo, CONEXIONES_PERMITIDAS);

	while(TRUE){

	  FD_ZERO(&readfds);
	  FD_SET(fdNucleo, &readfds);
	  max_fd = fdNucleo;

	  for ( i = 0 ; i < list_size(listaCPU) ; i++){

		  cpu * unCPU = (cpu *)list_get(listaCPU, i);
		  fd = unCPU->fd_cpu;
		  if(fd > 0)
			  FD_SET( fd , &readfds);
		  if(fd > max_fd)
		      max_fd = fd;
		    }

	  seleccionarSocket(max_fd, &readfds , NULL , NULL , NULL, NULL);

	if (FD_ISSET(fdNucleo, &readfds)) {
		  fdNuevoCPU = aceptarConexionSocket(fdNucleo);

		  int ret_handshake = handshake_servidor(fdNuevoCPU, "N");
		  	    if(ret_handshake == FALSE){
		  	    		perror("[ERROR] Se espera conexión del proceso CPU\n");
		  	    		cerrarSocket(fdNuevoCPU);
		  	    	}
		  	    	else{
		    	// Nueva conexión de CPU:
		    cpu * nuevoCPU = malloc(sizeof(cpu));

		    		nuevoCPU->id = fdNuevoCPU - fdNucleo;
		    		nuevoCPU->fd_cpu = fdNuevoCPU;
		    		nuevoCPU->disponibilidad = LIBRE;

		    		list_add(listaCPU, nuevoCPU);
		    		log_info(logger,"La CPU %i se ha conectado", nuevoCPU->id);

		    		planificarProceso();

		    	}
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
		          pcb * pcbDeRafaga =(pcb *) mensaje;
		          log_info(logger, "CPU %i - Programa AnSISOP %i - Fin de ráfaga", unCPU->id, pcbDeRafaga->pid);

		          actualizarDatosDePcbEjecutada(unCPU, pcbDeRafaga);

		          planificarProceso();

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

	log_destroy(logger);
	logger = NULL;
}

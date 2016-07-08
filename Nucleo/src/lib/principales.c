#include "principales.h"

/*******************************
 *    FUNCIONES PRINCIPALES    *
 ******************************/
void crearLoggerNucleo(){
	char * archivoLogNucleo = strdup("NUCLEO_LOG.log");
	logger = log_create("NUCLEO_LOG.log", archivoLogNucleo, true, LOG_LEVEL_INFO);
	free(archivoLogNucleo); archivoLogNucleo = NULL;
}

void inicializarColecciones(){
	listaCPU = list_create();
	listaConsolas = list_create();
	listaProcesos = list_create(); // Lista de todos los procesos en el sistema
	colaListos = queue_create();
	diccionarioIO = dictionary_create();
	diccionarioSemaforos = dictionary_create();
	diccionarioVarCompartidas = dictionary_create();
}

#define registrarConNombreYValor(functionName,type,dictionary,creator)\
				void functionName(char* name,int value) { \
						type *var = creator(name,value);\
						dictionary_put(dictionary, var->nombre, var);\
				}\

registrarConNombreYValor(registrarSemaforo, t_semaforo, diccionarioSemaforos, semaforo_create)
registrarConNombreYValor(registrarVariableCompartida, var_compartida, diccionarioVarCompartidas, crearVariableCompartida)

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

var_compartida* crearVariableCompartida(char* nombre, int valorInicial){

var_compartida* var = malloc(sizeof(var_compartida));
  var->nombre = strdup(nombre);
  var->valor = valorInicial;

  return var;
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

void activarConexionConConsolasYCPUs(){
	fdEscuchaConsola = nuevoSocket();
	asociarSocket(fdEscuchaConsola, config->puertoPrograma);
	escucharSocket(fdEscuchaConsola, CONEXIONES_PERMITIDAS);

	fdEscuchaCPU = nuevoSocket();
	asociarSocket(fdEscuchaCPU, config->puertoCPU);
	escucharSocket(fdEscuchaCPU, CONEXIONES_PERMITIDAS);
}

void esperar_y_PlanificarProgramas(){

		int i, fd, max_fd;

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
	    } // fin for consola

	    // Reviso si el fd de algún CPU supera al max_fd actual:
	    for ( i = 0 ; i < list_size(listaCPU) ; i++){

	    	cpu * unCPU = (cpu *)list_get(listaCPU, i);
	    	fd = unCPU->fd_cpu;
	    	if(fd > 0)
	    	FD_SET( fd , &readfds);
	    	if(fd > max_fd)
	    	max_fd = fd;
	     } // fin for cpu

	    seleccionarSocket(max_fd, &readfds , NULL , NULL , NULL, NULL);

	if (FD_ISSET(fdEscuchaConsola, &readfds) || FD_ISSET(fdEscuchaCPU, &readfds)) {

		if (FD_ISSET(fdEscuchaCPU, &readfds)) aceptarConexionEntranteDeCPU();

		if (FD_ISSET(fdEscuchaConsola, &readfds)) aceptarConexionEntranteDeConsola();
	}
	else{
	    		atenderNuevoMensajeDeCPU();
	    }
} // fin select

void aceptarConexionEntranteDeConsola(){

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
			  string * nuevoPrograma = (string*) entrada;
			  free(entrada);
		    nuevaConsola->programa.cadena = strdup(nuevoPrograma->cadena);
		    nuevaConsola->programa.tamanio = nuevoPrograma->tamanio;
		    	free(nuevoPrograma->cadena);
		    	free(nuevoPrograma);

		    	// Creo la PCB asociada a ese programa:
		    pcb * nuevoPcb = crearPcb(nuevaConsola->programa.cadena);
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

void atenderNuevoMensajeDeCPU(){
	int i, fd;

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

		string* variable = (string*)malloc(sizeof(string));
		variable->cadena = string_itoa(*((int*) mensaje));
		variable->tamanio = CHAR * string_length(variable->cadena + 1);

		bool consolaTieneElPid(void* unaConsola){ return (((consola*) unaConsola)->pid) == unCPU->pid;}

		consola * consolaAsociada = list_find(listaConsolas, (void *)consolaTieneElPid);

		// Le mando el msj a la Consola asociada:
		aplicar_protocolo_enviar(consolaAsociada->fd_consola, IMPRIMIR, variable);
			free(variable->cadena);
			free(variable);

		break;
						}
	case IMPRIMIR_TEXTO:{

		bool consolaTieneElPid(void* unaConsola){ return (((consola*) unaConsola)->pid) == unCPU->pid;}
		consola * consolaAsociada = list_find(listaConsolas, (void *)consolaTieneElPid);

		// Le mando el msj a la Consola asociada:
		aplicar_protocolo_enviar(consolaAsociada->fd_consola, IMPRIMIR_TEXTO, mensaje);

		break;
			   		   	 }
	case ABORTO_PROCESO:{

		int* pid = (int*)mensaje;
		int index = pcbListIndex(*pid);
		// Le informo a UMC:
		finalizarPrograma(*pid, index);

		bool consolaTieneElPid(void* unaConsola){ return (((consola*) unaConsola)->pid) == *pid;}
		consola * consolaAsociada = list_remove_by_condition(listaConsolas, consolaTieneElPid);

		// Le informo a la Consola asociada:
		aplicar_protocolo_enviar(consolaAsociada->fd_consola, FINALIZAR_PROGRAMA, NULL);
		liberarConsola(consolaAsociada);

		// Quito el PCB del sistema:
		liberarPcb(list_remove(listaProcesos, index));
		free(pid);

		break;
			            }
	case SIGNAL_REQUEST:{

		char* sem_id = (char*)mensaje;
		t_semaforo* semaforo = dictionary_get(diccionarioSemaforos, sem_id);
		semaforo_signal(semaforo);
		free(sem_id);

		break;
					  }
	case WAIT_REQUEST:{

		char* sem_id = (char*)mensaje;
		t_semaforo* semaforo = dictionary_get(diccionarioSemaforos, sem_id);
		free(sem_id);

		if (semaforo_wait(semaforo)){
		// WAIT NO OK: El proceso se bloquea, etonces tomo su pcb.

			aplicar_protocolo_enviar(fd, WAIT_CON_BLOQUEO, NULL);

			pcb* waitPcb = NULL;
			int head;

			void* entrada = aplicar_protocolo_recibir(fd, &head);
			if(head == PCB_EN_ESPERA){
				waitPcb = (pcb*)entrada;
			}

		// El proceso cambia de Ejecutando a Bloqueado:
			// --> Lo hace CPU antes de mandar

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

		    		int* quantum = (int*)malloc(INT);
		    		*quantum = config->quantum;
		    		aplicar_protocolo_enviar(nuevoCPU->fd_cpu, QUANTUM_MODIFICADO, quantum);
		    		free(quantum);

		    		list_add(listaCPU, nuevoCPU);
		    		log_info(logger,"La CPU %i se ha conectado", nuevoCPU->id);

		    	// Pongo al nuevo CPU a ejecutar un proceso:
		    		planificarProceso();
		    	}
}

void liberarMemoriaUtilizada(){
	limpiarColecciones();
	limpiarArchivoConfig();
	log_destroy(logger);
	logger = NULL;
}

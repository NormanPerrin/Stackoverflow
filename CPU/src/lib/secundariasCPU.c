#include "secundariasCPU.h"


// Funciones
void setearValores_config(t_config * archivoConfig){
	config = (t_configuracion*)reservarMemoria(sizeof(t_configuracion));
	config->ipNucleo = strdup(config_get_string_value(archivoConfig, "IP_NUCLEO"));
	config->ipUMC = strdup(config_get_string_value(archivoConfig, "IP_UMC"));
	config->puertoNucleo = config_get_int_value(archivoConfig, "PUERTO_NUCLEO");
	config->puertoUMC = config_get_int_value(archivoConfig, "PUERTO_UMC");
}

// --LOGGER--
void crearLogger(){
	char * archivoLogCPU = strdup("CPU_LOG.log");
	logger = log_create("CPU_LOG.log", archivoLogCPU, TRUE, LOG_LEVEL_INFO);
	free(archivoLogCPU);
	archivoLogCPU = NULL;
}

void ejecutarInstruccion(pcb* pcb){
	direccion direccionInstruccion;
	void * entrada = NULL;
	int protocolo;
	respuestaPedido * respuesta = NULL;

	direccionInstruccion.pagina = pcb->indiceCodigo[pcb->pc]; // falta obtener el numero de pagina!!
	direccionInstruccion.offset = pcb->indiceCodigo->offset;
	direccionInstruccion.size = pcb->tamanioIndiceCodigo;

	aplicar_protocolo_enviar(fd_clienteUMC, PEDIDO_LECTURA, &direccionInstruccion);

	entrada = aplicar_protocolo_recibir(fd_clienteUMC, &protocolo);

	if (protocolo == RESPUESTA_PEDIDO){
		respuesta = (respuestaPedido *) entrada;
		if(respuesta->estadoPedido==PERMITIDO){
				(pcb->pc)++; //incremento Program Counter del PCB
				char* instruccion = (char*)malloc(respuesta->dataPedida.tamanio);
				instruccion = strdup(respuesta->dataPedida.cadena);

				analizadorLinea(instruccion, &funcionesAnSISOP, &funcionesKernel);

				free(instruccion);
				free(respuesta->mensaje.cadena);
				free(respuesta->dataPedida.cadena);
				free(respuesta);
//				unsleep(pcb->retardoQuantum); //ya deberia estar en milisegundos

			}
			else{
				// UMC arrojó excepción:
				char* msjExcepcion = NULL;
				msjExcepcion = strdup(respuesta->mensaje.cadena);
				printf("%s", msjExcepcion);

				// es abortiva, ver qué se hace con la excepción recibida y con la ejecución
				/* Al terminar:
				 * free(respuesta->mensaje.cadena);
				 * free(respuesta->dataPedida.cadena);
				 * free(respuesta);*/
			}
		}else{
			log_error(logger, "Se esperaba una respuesta de lectura y no se recibió.");
			//abort();
		}
}

void ejecutarProceso(pcb* pcb){

	int quantum = pcb->quantum;
	int estado = pcb->estado;

	while(quantum > 0){
		ejecutarInstruccion(pcb);
		quantum--;
	}
	if(quantum == 0){
		estado = READY;
		pcb->estado = estado;
	}

	aplicar_protocolo_enviar(fd_clienteNucleo, FIN_QUANTUM, NULL);
	aplicar_protocolo_enviar(fd_clienteNucleo, PCB, pcb);

}

int validar_servidor(char *id) {
	if( !strcmp(id, "U") || !strcmp(id, "N") ) {
		printf("Servidor aceptado\n");
		return TRUE;
	} else {
		printf("Servidor rechazado\n");
		return FALSE;
	}
}

int validar_cliente(char *id) {return 0;}


//	free(config->ipNucleo);



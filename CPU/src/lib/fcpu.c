#include "fcpu.h"


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

void conectarConUMC(){
	fd_clienteUMC = nuevoSocket();
	int ret = conectarSocket(fd_clienteUMC, config->ipUMC, config->puertoUMC);
	validar_conexion(ret, 1);
	handshake_cliente(fd_clienteUMC, "P");

	int * tamPagina = (int*)malloc(INT);
	recibirPorSocket(fd_clienteUMC, tamPagina, INT);
	tamanioPagina = *tamPagina; // setea el tamaño de pág. que recibe de UMC

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

	entrada = aplicar_protocolo_recibir(fd_clienteUMC, protocolo);

	if (protocolo == RESPUESTA_PEDIDO){
		respuesta = (respuestaPedido *) entrada;
		if(respuesta->estadoPedido==PERMITIDO){
				(pcb->pc)++; //incremento Program Counter del PCB
				char* instruccion = (char*)malloc(respuesta->dataPedida.tamanio);
				instruccion = strdup(respuesta->dataPedida.cadena);

				analizadorLinea(instruccion, &funcionesAnSISOP, &funcionesKernel);
				// ver qué se hace con la instrucción
				/* Al terminar:
				* free(respuesta->mensaje.cadena);
				*  free(respuesta->dataPedida.cadena);
				* free(respuesta);*/

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

void conectarConNucleo() {
	fd_clienteNucleo = nuevoSocket();
	int ret = conectarSocket(fd_clienteNucleo, config->ipNucleo, config->puertoNucleo);
	validar_conexion(ret, 1); // Es terminante por ser cliente
	handshake_cliente(fd_clienteNucleo, "P");
	int protocolo;

	void * mensaje = aplicar_protocolo_recibir(fd_clienteNucleo, protocolo);
	if (protocolo == PCB){
		while(mensaje!=NULL){
				// El CPU obtiene una PCB para ejecutar:
				pcb * pcbEnEjecucion = malloc(sizeof(pcb));
				memcpy(pcbEnEjecucion, mensaje, sizeof(pcb));
				free(mensaje);

				ejecutarProceso(pcbEnEjecucion);
			}
	}else{
			log_error(logger, "Se esperaba un PCB y no se recibió.");
				//abort();
			}

	cerrarSocket(fd_clienteUMC);
	cerrarSocket(fd_clienteNucleo);
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

void liberarEstructura() {
	free(config->ipNucleo);
	free(config->ipUMC);
	free(config);
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


/*void esperar_ejecucion() {

	uint8_t *head = (uint8_t*)reservarMemoria(1); // 0 .. 255

	while(TRUE) {
		int ret;
		ret = recibirPorSocket(fd_clienteNucleo, head, INT);
		validar_recive(ret, 1); // es terminante ya que si hay un error en el recive o desconexión debe terminar
		aplicar_protocolo_recibir(fd_clienteNucleo, *head);
	}
	free(head);
}*/

// Primitivas AnSISOP
t_puntero definirVariable(t_nombre_variable identificador_variable){
	t_puntero posicion;

	return posicion;
}

t_puntero obtenerPosicionVariable(t_nombre_variable identificador_variable){
	t_puntero posicion;

	return posicion;
}

t_valor_variable dereferenciar(t_puntero direccion_variable){
	t_valor_variable valorVariable;

	return valorVariable;
}

void asignar(t_puntero direccion_variable, t_valor_variable valor){

}

t_valor_variable obtenerValorCompartida(t_nombre_compartida variable){
	t_valor_variable valorVariable;

	return valorVariable;
}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){
	t_valor_variable valorVariable;

	return valorVariable;
}

void irAlLabel(t_nombre_etiqueta t_nombre_etiqueta){

}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){

}

void retornar(t_valor_variable retorno){

}

void imprimirAnSISOP(t_valor_variable valor_mostrar){

}

void imprimirTexto(char* texto){

}

void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo){

}

void wait(t_nombre_semaforo identificador_semaforo){

}

void signal(t_nombre_semaforo identificador_semaforo){

}

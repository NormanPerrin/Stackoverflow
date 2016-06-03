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

	direccion* unaDireccion = (direccion*) malloc(sizeof(direccion));
	unaDireccion->pagina = pcb->indiceCodigo.instrucciones[pcb->pc]; // falta obtener el numero de pagina!!
	unaDireccion->offset = pcb->indiceCodigo.instrucciones->offset;
	unaDireccion->size = pcb->indiceCodigo.tamanio;

	aplicar_protocolo_enviar(fd_clienteUMC, PEDIDO_LECTURA, unaDireccion,sizeof(direccion));

	respuestaPedido * respuesta = aplicar_protocolo_recibir(fd_clienteUMC, RESPUESTA_PEDIDO, TAMANIO_BASE);

	if(respuesta->estadoPedido==PERMITIDO){
		(pcb->pc)++; //incremento Program Counter del PCB
		char* const instruccion = (char* const)malloc(respuesta->dataPedida.tamanio);
		*instruccion = respuesta->dataPedida.cadena;

		analizadorLinea(instruccion, &funcionesAnSISOP, &funcionesKernel);
		//free(instruccion); o ver antes qué se hace con ella
	}
	else{
		// UMC arrojó excepción:
		char* msjExcepcion = (char*)malloc(respuesta->mensaje.tamanio);
		*msjExcepcion = respuesta->mensaje;
		// ver qué se hace con la excepción recibida
		// free(msjExcepcion); al terminar
	}

	free(respuesta);
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
	void * mensaje;
	aplicar_protocolo_enviar(fd_clienteNucleo, FIN_QUANTUM, mensaje, INT);
	aplicar_protocolo_enviar(fd_clienteNucleo, PCB, pcb, TAMANIO_BASE);

}


void conectarConNucleo() {
	fd_clienteNucleo = nuevoSocket();
	int ret = conectarSocket(fd_clienteNucleo, config->ipNucleo, config->puertoNucleo);
	validar_conexion(ret, 1); // Es terminante por ser cliente
	handshake_cliente(fd_clienteNucleo, "P");

	void * mensaje = aplicar_protocolo_recibir(fd_clienteNucleo, PCB, TAMANIO_BASE);

	while(mensaje!=NULL){
		// El CPU obtiene una PCB para ejecutar:
		pcb * pcbEnEjecucion = malloc(sizeof(pcb));
		memcpy(pcbEnEjecucion, mensaje, sizeof(pcb));

		// Desarrollar alguna función que permita ejecutar la PCB
		ejecutarProceso(pcbEnEjecucion->estado);
	}
	free(mensaje);

	cerrarSocket(fd_clienteUMC);
	cerrarSocket(fd_clienteNucleo);
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

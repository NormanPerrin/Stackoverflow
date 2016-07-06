#include "primitivasAnSISOP.h"
#include "principalesCPU.h"

t_puntero AnSISOP_definirVariable(t_nombre_variable var_nombre){

	/* Le asigna una posición en memoria a la variable,
	 y retorna el offset total respecto al inicio del stack. */

	int var_indiceStack_posicion = pcbActual->ultimaPosicionIndiceStack;
	registroStack registroActual = pcbActual->indiceStack[var_indiceStack_posicion];

	char * var_id = charAString(var_nombre);

	direccion * var_direccion = malloc(sizeof(direccion));
	var_direccion->pagina = 0;
	var_direccion->offset = pcbActual->stackPointer;
	var_direccion->size = INT;

		while(var_direccion->offset > tamanioPagina){
			(var_direccion->pagina)++;
			var_direccion->offset -= tamanioPagina;
		}

	// pcbActual->stackPointer: offset total de la última posición disponible en el stack de memoria

	dictionary_put(registroActual.variables, var_id, var_direccion);
	free (var_id);
	free(var_direccion);

	int var_stack_offset = pcbActual->stackPointer;
	pcbActual->stackPointer += INT;

	return var_stack_offset;
}

t_puntero AnSISOP_obtenerPosicionVariable(t_nombre_variable var_nombre){

	/* En base a la posición de memoria de la variable,
	 retorna el offset total respecto al inicio del stack. */

	int var_indiceStack_posicion = pcbActual->ultimaPosicionIndiceStack -1;
	registroStack registroActual = pcbActual->indiceStack[var_indiceStack_posicion];

	char* var_id = charAString(var_nombre);

	direccion * var_direccion = malloc(sizeof(direccion));
	var_direccion = (direccion*)dictionary_get(registroActual.variables, var_id);
	free(var_id);

	if(var_direccion == NULL){
		free(var_direccion);

		return ERROR;
	}
	else{
		int var_stack_offset = (var_direccion->pagina * tamanioPagina) + var_direccion->offset;

		return var_stack_offset;
	}
}

t_valor_variable AnSISOP_dereferenciar(t_puntero var_stack_offset){

	// Retorna el valor leído a partir de var_stack_offset.

	solicitudLectura * var_direccion = malloc(sizeof(solicitudLectura));

	int num_pagina =  var_stack_offset / tamanioPagina;
	int offset = var_stack_offset - (num_pagina*tamanioPagina);
		var_direccion->pagina = num_pagina;
		var_direccion->offset = offset;
		var_direccion->tamanio = INT;

	int head;
	void* entrada = NULL;
	int* valor_variable = NULL;

	aplicar_protocolo_enviar(fdUMC, PEDIDO_LECTURA_VARIABLE, var_direccion);

	entrada = aplicar_protocolo_recibir(fdUMC, &head);

	recibirYvalidarEstadoDelPedidoAUMC();

	entrada = aplicar_protocolo_recibir(fdUMC, &head);

	if(head == DEVOLVER_VARIABLE){
		valor_variable = (int*)entrada;
	}
	else{
		printf("Error al leer una variable del proceso #%d", pcbActual->pid);
	}
		free(entrada);
		int var_valor = *valor_variable;
		free(valor_variable);

	return var_valor;
}

void AnSISOP_asignar(t_puntero var_stack_offset, t_valor_variable valor){

	// Escribe en memoria el valor en la posición dada.

	solicitudEscritura * var_escritura = malloc(sizeof(solicitudEscritura));

	int num_pagina =  var_stack_offset / tamanioPagina;
	int offset = var_stack_offset - (num_pagina*tamanioPagina);
		var_escritura->pagina = num_pagina;
		var_escritura->offset = offset;
		var_escritura->tamanio = INT;
		var_escritura->contenido = valor;

		aplicar_protocolo_enviar(fdUMC, PEDIDO_ESCRITURA, var_escritura);
		free(var_escritura),

		recibirYvalidarEstadoDelPedidoAUMC();
}


t_valor_variable AnSISOP_obtenerValorCompartida(t_nombre_compartida var_compartida_nombre){

	var_compartida * variableCompartida = malloc(sizeof(var_compartida));
	void* entrada = NULL;
	int* valor_variable = NULL;
	int head;

	variableCompartida->nombre = (char*) var_compartida_nombre;

	aplicar_protocolo_enviar(fdNucleo, OBTENER_VAR_COMPARTIDA, variableCompartida);
	free(variableCompartida);

	entrada = aplicar_protocolo_recibir(fdNucleo, &head);
	if(head == DEVOLVER_VAR_COMPARTIDA){
		valor_variable = (int*) entrada;
	}
	else{
		printf("Error al obtener variable compartida del proceso #%d", pcbActual->pid);
	}

	free(entrada);
	int valorVariable = *valor_variable;
	free(valor_variable);

	return valorVariable;

}

t_valor_variable AnSISOP_asignarValorCompartida(t_nombre_compartida var_compartida_nombre, t_valor_variable var_compartida_valor){

	var_compartida * variableCompartida = malloc(sizeof(var_compartida));
	void* entrada = NULL;
	int* valor_variable = NULL;
	int head;

	variableCompartida->nombre = (char*) var_compartida_nombre;
	variableCompartida->valor = (int) var_compartida_valor;

	aplicar_protocolo_enviar(fdNucleo, GRABAR_VAR_COMPARTIDA, variableCompartida);
	free(variableCompartida);

	entrada = aplicar_protocolo_recibir(fdNucleo, &head);
	if(head == DEVOLVER_VAR_COMPARTIDA){
		valor_variable = (int*) entrada;
	}
	else{
		printf("Error al asignar variable compartida del proceso #%d", pcbActual->pid);
	}

	free(entrada);
	int valorVariable = *valor_variable;
	free(valor_variable);

	return valorVariable;
}

//HACER
void AnSISOP_irAlLabel(t_nombre_etiqueta nombre_etiqueta){

}

//HACER
void AnSISOP_llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){

}

//HACER
void AnSISOP_retornar(t_valor_variable retorno){

}

//ARREGLAR
void AnSISOP_imprimir(t_valor_variable valor_mostrar){
	int * valor = malloc(INT);
	* valor = valor_mostrar;
	aplicar_protocolo_enviar(fdNucleo,IMPRIMIR, valor);
	free(valor);
}

//HECHA(REVISAR SI ESTA BIEN)
void AnSISOP_imprimirTexto(char* texto){
	string * txt = malloc(STRING);
	txt->cadena = strdup(texto);
	txt->tamanio = strlen(texto) + 1;
	aplicar_protocolo_enviar(fdNucleo, IMPRIMIR_TEXTO, txt);
	free(txt->cadena);
	free(txt);
}

void AnSISOP_entradaSalida(t_nombre_dispositivo nombre_dispositivo, int tiempo){

	pedidoIO * pedidoEntradaSalida = malloc(sizeof(pedidoEntradaSalida));
	pedidoEntradaSalida->nombreDispositivo = (char*) nombre_dispositivo;
	pedidoEntradaSalida->tiempo = tiempo;

	aplicar_protocolo_enviar(fdNucleo,ENTRADA_SALIDA, pedidoEntradaSalida);
	log_info(logger, "Proceso %i utiliza dispositivo I/O : %s durante %i", pcbActual->pid,(char*)nombre_dispositivo,tiempo);
	free(pedidoEntradaSalida);

}

void AnSISOP_wait(t_nombre_semaforo identificador_semaforo){

	char* id_semaforo = malloc(sizeof(char*));
	id_semaforo = identificador_semaforo;
	int head;
	void* entrada = NULL;

	aplicar_protocolo_enviar(fdNucleo,WAIT_REQUEST,id_semaforo);
	free(id_semaforo);

	entrada = aplicar_protocolo_recibir(fdNucleo, &head);

	if(head == WAIT_CON_BLOQUEO){
		pcbActual->estado = BLOCK;
		aplicar_protocolo_enviar(fdNucleo, PCB_EN_ESPERA, pcbActual);
		log_info(logger, "El proceso %i queda bloqueado al hacer WAIT", pcbActual->pid);
	}
	if(head == WAIT_SIN_BLOQUEO){
		log_info(logger, "El proceso %i sigue ejecutando correctamente al hacer WAIT", pcbActual->pid);
	}
	free(entrada);
}

void AnSISOP_signal(t_nombre_semaforo identificador_semaforo){

	char* id_semaforo = malloc(sizeof(char*));
	id_semaforo = identificador_semaforo;

	aplicar_protocolo_enviar(fdNucleo,SIGNAL_REQUEST,id_semaforo);
	log_info(logger, "SIGNAL en el proceso %i", pcbActual->pid);
	free(id_semaforo);

}


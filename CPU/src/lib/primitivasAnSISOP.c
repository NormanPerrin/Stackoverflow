#include "primitivasAnSISOP.h"

t_puntero definirVariable(t_nombre_variable var_nombre){

	/* Le asigna una posición en memoria a la variable,
	 y retorna el offset total respecto al inicio del stack. */

	int ultima_posicion_indiceStack = pcbActual->ultimaPosicionIndiceStack;
	t_list* indStack = pcbActual->indiceStack->elements;
	registroStack* registroActual= list_get(indStack,ultima_posicion_indiceStack);


	char * var_id = strdup(charAString(var_nombre));

	direccion * var_direccion = malloc(sizeof(direccion));
	var_direccion->pagina = 0;
	var_direccion->offset = pcbActual->stackPointer;
	var_direccion->size = INT;

		while(var_direccion->offset > tamanioPagina){
			(var_direccion->pagina)++;
			var_direccion->offset -= tamanioPagina;
		}

	// pcbActual->stackPointer: offset total de la última posición disponible en el stack de memoria

	dictionary_put(registroActual->vars, var_id, var_direccion);
	free (var_id);
	free(var_direccion);

	int var_stack_offset = pcbActual->stackPointer;
	pcbActual->stackPointer += INT;

	return var_stack_offset;
}

t_puntero obtenerPosicionVariable(t_nombre_variable var_nombre){

	/* En base a la posición de memoria de la variable,
	 retorna el offset total respecto al inicio del stack. */

	int ultima_posicion_indiceStack = pcbActual->ultimaPosicionIndiceStack -1;
	t_list* indStack = pcbActual->indiceStack->elements;
	registroStack* registroActual= list_get(indStack,ultima_posicion_indiceStack);

	char* var_id = strdup(charAString(var_nombre));

	direccion * var_direccion = malloc(sizeof(direccion));
	var_direccion = (direccion*)dictionary_get(registroActual->vars, var_id);
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

t_valor_variable dereferenciar(t_puntero var_stack_offset){
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

void asignar(t_puntero var_stack_offset, t_valor_variable valor){

	// Escribe en memoria el valor en la posición dada.

	solicitudEscritura * var_escritura = malloc(sizeof(solicitudEscritura));

	int num_pagina =  var_stack_offset / tamanioPagina;
	int offset = var_stack_offset - (num_pagina*tamanioPagina);
		var_escritura->pagina = num_pagina;
		var_escritura->offset = offset;
		var_escritura->contenido = valor;

		aplicar_protocolo_enviar(fdUMC, PEDIDO_ESCRITURA, var_escritura);
		free(var_escritura),

		recibirYvalidarEstadoDelPedidoAUMC();
}

t_valor_variable obtenerValorCompartida(t_nombre_compartida var_compartida_nombre){

	char * variableCompartida = malloc(strlen(var_compartida_nombre)+1);
	void* entrada = NULL;
	int* valor_variable = NULL;
	int head;

	variableCompartida = strdup((char*) var_compartida_nombre);

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
	int valor = *valor_variable;
	free(valor_variable);

	return valor;
}

t_valor_variable asignarValorCompartida(t_nombre_compartida var_compartida_nombre, t_valor_variable var_compartida_valor){

	var_compartida * variableCompartida = malloc(sizeof(var_compartida));

	variableCompartida->nombre = strdup((char*) var_compartida_nombre);
	variableCompartida->valor = var_compartida_valor;

	aplicar_protocolo_enviar(fdNucleo, GRABAR_VAR_COMPARTIDA, variableCompartida);
	free(variableCompartida->nombre);
	free(variableCompartida);

	return var_compartida_valor;
}

t_puntero_instruccion irAlLabel(t_nombre_etiqueta nombre_etiqueta){

	t_puntero_instruccion next_pc = metadata_buscar_etiqueta(nombre_etiqueta, pcbActual->indiceEtiquetas, pcbActual->tamanioIndiceEtiquetas);

	return next_pc;
}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){


	uint32_t tamRegistroStack = 4*sizeof(uint32_t)+2*sizeof(t_list);
	registroStack* nuevoRegistroStack = malloc(sizeof(tamRegistroStack));
	direccion* varRetorno = malloc(sizeof(direccion));
	int num_pagina =  donde_retornar / tamanioPagina;
	int offset = donde_retornar % tamanioPagina;

	varRetorno->pagina = num_pagina;
	varRetorno->offset = offset;
	varRetorno->size = INT;

	nuevoRegistroStack->args = NULL;
	nuevoRegistroStack->vars = dictionary_create();
	nuevoRegistroStack->retVar = *varRetorno;
	nuevoRegistroStack->retPos = pcbActual->pc;
	stack_push(pcbActual->indiceStack, nuevoRegistroStack);

	pcbActual->numeroContextoEjecucionActualStack++;

	free(varRetorno);
	free(nuevoRegistroStack->args);
	free(nuevoRegistroStack->vars);
	free(nuevoRegistroStack);

	irAlLabel(etiqueta);

}

void retornar(t_valor_variable retorno){

	//Agarro contexto actual y anterior
	int numeroEjecucionActual=pcbActual->numeroContextoEjecucionActualStack;
	uint32_t tamRegistroStack = 4*sizeof(uint32_t)+2*sizeof(t_list);
	registroStack* contextoEjecucionActual = malloc(sizeof(tamRegistroStack));

	//Limpio el contexto de ejecucion actual
	int tamanioArgs = list_size(contextoEjecucionActual->args);//ARREGLAR
	int i;
	for (i=0; i < tamanioArgs; i++){
		direccion* argumento = malloc(sizeof(direccion));
		argumento = list_get(contextoEjecucionActual->args,i);//ARREGLAR
		pcbActual->stackPointer = pcbActual->stackPointer-4;
		free(argumento);
	}
	int tamanioVars = dictionary_size(contextoEjecucionActual->vars);
	for(i=0; i < tamanioVars; i++){
		t_dictionary * variable = malloc(sizeof(t_dictionary));
		variable = dictionary_get(contextoEjecucionActual->vars,i); //ARREGLAR(NO LA POSICION SINO LA CLAVE)
		pcbActual->stackPointer=pcbActual->stackPointer-4;
		free(variable);
			}

		direccion* retVar = malloc(sizeof(direccion));
		*retVar = contextoEjecucionActual->retVar;
		t_puntero direcVariable = (retVar->pagina*tamanioPagina)+retVar->offset;
		free(retVar);

		//calculo la direccion a la que tengo que retornar mediante la direccion de pagina start y offset que esta en el campo retVar
		asignar(direcVariable,retorno);

		//Elimino el contexto actual del indice del stack
		//Seteo el contexto de ejecucion actual en el anterior

		pcbActual->pc = contextoEjecucionActual->retPos;
		free(contextoEjecucionActual);
		list_remove(pcbActual->indiceStack->elements,pcbActual->numeroContextoEjecucionActualStack);
		pcbActual->numeroContextoEjecucionActualStack-=1;
		t_stack * contextoEjecNuevo= list_get (pcbActual->indiceStack->elements,pcbActual->numeroContextoEjecucionActualStack);
		log_debug(logger,"Llamada a retornar" );
		return;
}

void imprimir(t_valor_variable valor_mostrar){

	int * valor = malloc(INT);
	*valor = valor_mostrar;

	aplicar_protocolo_enviar(fdNucleo,IMPRIMIR, valor);

	free(valor);
}

void imprimirTexto(char* texto){

	string * txt = malloc(STRING);
	txt->cadena = strdup(texto);
	txt->tamanio = strlen(texto) + 1;

	aplicar_protocolo_enviar(fdNucleo, IMPRIMIR_TEXTO, txt);

	free(txt->cadena);
	free(txt);
}

void entradaSalida(t_nombre_dispositivo nombre_dispositivo, int tiempo){

	pedidoIO * pedidoEntradaSalida = malloc(strlen(nombre_dispositivo)+ 1+ INT);
	pedidoEntradaSalida->nombreDispositivo = strdup((char*) nombre_dispositivo);
	pedidoEntradaSalida->tiempo = tiempo;

	aplicar_protocolo_enviar(fdNucleo,ENTRADA_SALIDA, pedidoEntradaSalida);

	log_info(logger, "Proceso %i utiliza dispositivo I/O: %s durante %i unidades de tiempo.", pcbActual->pid,nombre_dispositivo,tiempo);
	free(pedidoEntradaSalida->nombreDispositivo);
	free(pedidoEntradaSalida);
}

void s_wait(t_nombre_semaforo identificador_semaforo){

	char* id_semaforo = malloc(strlen(identificador_semaforo)+1);
	id_semaforo = strdup((char*)identificador_semaforo);
	int head;
	void* entrada = NULL;

	aplicar_protocolo_enviar(fdNucleo,WAIT_REQUEST,id_semaforo);
	free(id_semaforo);

	entrada = aplicar_protocolo_recibir(fdNucleo, &head);

	if(head == WAIT_CON_BLOQUEO){
		// Mando la pcb bloqueada y la saco de ejecución:
		// TODO: Ver el tipo de mensaje
		aplicar_protocolo_enviar(fdNucleo, PCB_WAIT, pcbActual);
		log_info(logger, "El proceso %i queda bloqueado al hacer WAIT", pcbActual->pid);
		liberarPcbActiva();
	}
	else{
		log_info(logger, "El proceso %i sigue ejecutando correctamente al hacer WAIT", pcbActual->pid);
	}
	free(entrada);
}

void s_signal(t_nombre_semaforo identificador_semaforo){

	char* id_semaforo = malloc(strlen(identificador_semaforo)+1);
	id_semaforo = strdup((char*)identificador_semaforo);

	aplicar_protocolo_enviar(fdNucleo,SIGNAL_REQUEST,id_semaforo);

	log_info(logger, "SIGNAL en el proceso %i", pcbActual->pid);
	free(id_semaforo);
}

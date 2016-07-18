#include "primitivasAnSISOP.h"

/* FUNCIONES EXTRA */

bool esArgumento(t_nombre_variable identificador_variable){
	if(isdigit(identificador_variable)){
		return true;
	}else{
		return false;
	}
}

/* --- PRIMITIVAS --- */
t_puntero definirVariable(t_nombre_variable var_nombre){

	if(!esArgumento(var_nombre)){
		printf("Definiendo nueva variable: '%c'.\n", var_nombre);
	}
	else{
		log_info(logger, "Definiendo nuevo argumento: '%c'.", var_nombre);
	}

		char * var_id = strdup(charAString(var_nombre));

		direccion * var_direccion = malloc(sizeof(direccion));
		var_direccion->pagina = pcbActual->primerPaginaStack;
		var_direccion->offset = pcbActual->stackPointer;
		var_direccion->size = INT;

		while(var_direccion->offset > tamanioPagina){
			(var_direccion->pagina)++;
			var_direccion->offset -= tamanioPagina;
		}
		// Verifico si se desborda la pila en memoria:
		if(pcbActual->stackPointer + 4 > (tamanioPagina*tamanioStack)){
				printf("Stack Overflow al definir variable.\n");
				huboStackOverflow = true;

			return ERROR;
		}else{
			// Agrego un nuevo registro al índice de stack:
		registroStack* regStack = list_get(pcbActual->indiceStack, pcbActual->indexActualStack);
			if(regStack == NULL){
				regStack = reg_stack_create(); // TODO: Ver si pasar tamaño(s) como argumento del creator
			}
			//(stackPointer: desplazamiento desde la primer página del stack hasta donde arranca mi nueva variable)
			if(!esArgumento(var_nombre)){
				variable* new_var = malloc(14);
				new_var->direccion.offset = var_direccion->offset;
				new_var->direccion.pagina = var_direccion->pagina;
				new_var->direccion.size = var_direccion->size;
				new_var->nombre = strdup(var_id);
				list_add(regStack->vars, new_var);
			}
			else{
				variable* new_arg = malloc(14);
				new_arg->direccion.offset = var_direccion->offset;
				new_arg->direccion.pagina = var_direccion->pagina;
				new_arg->direccion.size = var_direccion->size;
				new_arg->nombre = strdup(var_id);
				list_add(regStack->args, new_arg);
			}
			printf("'%c' -> Dirección lógica: %i, %i, %i.\n", var_nombre, var_direccion->pagina, var_direccion->offset,var_direccion->size);
			free (var_id); var_id = NULL;
			free(var_direccion); var_direccion = NULL;
			// Guardo la nueva variable en el índice:
			list_add(pcbActual->indiceStack, regStack);
			// Valor a retornar:
			int var_stack_offset = pcbActual->stackPointer;
			// Actualizo parámetros del PCB:
			pcbActual->stackPointer += INT;
			int total_heap_offset = ((pcbActual->paginas_codigo*tamanioPagina)+pcbActual->stackPointer);
			pcbActual->paginaActualStack = total_heap_offset/tamanioPagina;

		return var_stack_offset;
		} // fin else ERROR
}

t_puntero obtenerPosicionVariable(t_nombre_variable var_nombre){

	if(!esArgumento(var_nombre)){
		printf("Obteneniendo posición de la variable: '%c'.\n", var_nombre);
	}
	else{
		printf("Obteneniendo posición del argumento: '%c'.\n", var_nombre);
	}
	char* var_id = strdup(charAString(var_nombre));
	// Obtengo el registro del stack correspondiente al contexto de ejecución actual:
	registroStack* regStack = list_get(pcbActual->indiceStack, pcbActual->indexActualStack);
	// Me posiciono al inicio de este registro y busco la variable del diccionario que coincida con el nombre solicitado:

	if(!esArgumento(var_nombre)){ // Si entra acá es una variable:
		if(list_size(regStack->vars) > 0){

			direccion * var_direccion = malloc(sizeof(direccion));
			// Obtengo la variable buscada:
			int i;
			for(i = 0; i<list_size(regStack->vars); i++){

				variable* variable = list_get(regStack->vars, i);
				if(string_equals_ignore_case(variable->nombre, var_id)){

					free(var_id); var_id = NULL;

					int var_stack_page = var_direccion->pagina - pcbActual->primerPaginaStack;
					int var_stack_offset = (var_stack_page*tamanioPagina) + var_direccion->offset;
					free(var_direccion); var_direccion = NULL;

					return var_stack_offset;
				} // fin if equals
			} // fin for variables
		}
		printf("Error: No hay variables en el registro actual de stack.\n");
		return ERROR;
	} // Si entra acá es un argumento:
	else{
		if(list_size(regStack->args) > 0){

			direccion * var_direccion = malloc(sizeof(direccion));
			// Obtengo la variable buscada:
			int j;
			for(j = 0; j<list_size(regStack->args); j++){

				variable* argumento = list_get(regStack->args, j);
				if(string_equals_ignore_case(argumento->nombre, var_id)){

					free(var_id); var_id = NULL;

					int var_stack_page = var_direccion->pagina - pcbActual->primerPaginaStack;
					int var_stack_offset = (var_stack_page*tamanioPagina) + var_direccion->offset;
					free(var_direccion); var_direccion = NULL;

					return var_stack_offset;
				} // fin if equals
			} // fin for argumentos

		}
		printf("Error: No hay argumentos en el registro actual de stack.\n");
		return ERROR;
	} // fin else argumentos
}

t_valor_variable dereferenciar(t_puntero var_stack_offset){

	/* Retorna el valor leído a partir de var_stack_offset. */
	printf("Dereferenciando variable...\n");
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
	free(var_direccion); var_direccion = NULL;

	// Valido el pedido de lectura a UMC:
	if(!recibirYvalidarEstadoDelPedidoAUMC()){ // hubo error de lectura
		printf("Error: La variable no pudo dereferenciarse.\n");
		exitPorErrorUMC();
		return 0;
	}
	else{ // no hubo error de lectura
		entrada = aplicar_protocolo_recibir(fdUMC, &head); // respuesta OK de UMC, recibo la variable leída
		if(head == DEVOLVER_VARIABLE){
			valor_variable = (int*)entrada;
		}
		else{
			printf("Error al leer variable del proceso actual.\n");
		}
		return *valor_variable;
	}
}

void asignar(t_puntero var_stack_offset, t_valor_variable valor){

	/* Escribe en el stack de memoria el valor en la posición dada. */

	printf("Escribiendo variable...\n");
	solicitudEscritura * var_escritura = malloc(sizeof(solicitudEscritura));

	int num_pagina =  var_stack_offset / tamanioPagina;
	int offset = var_stack_offset - (num_pagina*tamanioPagina);
		var_escritura->pagina = num_pagina;
		var_escritura->offset = offset;
		var_escritura->contenido = valor;

	aplicar_protocolo_enviar(fdUMC, PEDIDO_ESCRITURA, var_escritura);
	free(var_escritura); var_escritura = NULL;

	// Valido el pedido de lectura a UMC:
	if(!recibirYvalidarEstadoDelPedidoAUMC()){
		printf("Error: La variable no pudo asignarse.\n");
		exitPorErrorUMC();
	}
	return;
}

t_valor_variable obtenerValorCompartida(t_nombre_compartida var_compartida_nombre){

	/* Solicita al Núcleo el valor de la variable compartida. */
	printf("Obteniendo el valor de la variable compartida: '%s'.\n", var_compartida_nombre);
	char * variableCompartida = malloc(strlen(var_compartida_nombre)+1);
	void* entrada = NULL;
	int* valor_variable = NULL;
	int head;

	variableCompartida = strdup((char*) var_compartida_nombre);

	aplicar_protocolo_enviar(fdNucleo, OBTENER_VAR_COMPARTIDA, variableCompartida);
	free(variableCompartida); variableCompartida = NULL;

	entrada = aplicar_protocolo_recibir(fdNucleo, &head);
	if(head == DEVOLVER_VAR_COMPARTIDA){
		valor_variable = (int*) entrada;
	}
	if(valor_variable == NULL){
		printf("Error al obtener variable compartida del proceso actual.\n");
		return ERROR;
	}
	else{
		return *valor_variable;
	}
}

t_valor_variable asignarValorCompartida(t_nombre_compartida var_compartida_nombre, t_valor_variable var_compartida_valor){

	printf("Asignando el valor %d a la variable compartida '%s'.\n", var_compartida_valor, var_compartida_nombre);
	var_compartida * variableCompartida = malloc(strlen(var_compartida_nombre)+ 5);

	variableCompartida->nombre = strdup((char*) var_compartida_nombre);
	variableCompartida->valor = var_compartida_valor;

	aplicar_protocolo_enviar(fdNucleo, GRABAR_VAR_COMPARTIDA, variableCompartida);
	free(variableCompartida->nombre); variableCompartida->nombre = NULL;
	free(variableCompartida); variableCompartida = NULL;

	return var_compartida_valor;
}

void irAlLabel(t_nombre_etiqueta nombre_etiqueta){
	printf("Llendo a la etiqueta: '%s'.\n", nombre_etiqueta);
	t_puntero_instruccion num_instruccion = metadata_buscar_etiqueta(nombre_etiqueta, pcbActual->indiceEtiquetas, pcbActual->tamanioIndiceEtiquetas);
	pcbActual->pc = num_instruccion - 1;
	return;
}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){
	/* Reserva espacio para un nuevo contexto vacío preservando el contexto
	 *  de ejecución actual, para luego volver al mismo. */
	printf("Llamada con retorno. Preservando contexto de ejecución actual y posición de retorno.\n");

	// Calculo la dirección de retorno y la guardo:
	registroStack * nuevoRegistroStack = reg_stack_create();
	nuevoRegistroStack->retVar.pagina = donde_retornar/tamanioPagina;
	nuevoRegistroStack->retVar.offset = donde_retornar%tamanioPagina;
	nuevoRegistroStack->retVar.size = INT;

	nuevoRegistroStack->retPos = pcbActual->pc; // Guardo el valor actual del program counter
	list_add(pcbActual->indiceStack, nuevoRegistroStack);

	pcbActual->indexActualStack++;

	irAlLabel(etiqueta);
	return;
}

void retornar(t_valor_variable var_retorno){

	printf("Llamada a la función 'retornar'.\n");
	// Tomo contexto actual y anterior:
	int index = pcbActual->indexActualStack;
	registroStack* registroActual = list_get(pcbActual->indiceStack, index);

	// Limpio los argumentos del registro y descuento el espacio que ocupan en el stack en memoria:
	pcbActual->stackPointer -= (4* list_size(registroActual->args));

	// Limpio las variables del registro y descuento el espacio que ocupan en el stack en memoria:
	pcbActual->stackPointer -= (4 * list_size(registroActual->vars));

	// Calculo la dirección de retorno a partir de retVar:
	t_puntero var_stack_offset = (registroActual->retVar.pagina * tamanioPagina) + registroActual->retVar.offset;
	asignar(var_stack_offset, var_retorno);

	// Elimino el contexto actual del índice de stack:
	// Luego, seteo el contexto de ejecución actual en el index anterior:
	pcbActual->pc =  registroActual->retPos;

	liberarRegistroStack(registroActual); // libero la memoria del registro
	free(list_remove(pcbActual->indiceStack, pcbActual->indexActualStack));
	pcbActual->indexActualStack--;
	return;
}

void imprimir(t_valor_variable valor_mostrar){
	printf("Solicitando imprimir variable.\n");
	int * valor = malloc(INT);
	*valor = valor_mostrar;
	aplicar_protocolo_enviar(fdNucleo,IMPRIMIR, valor);
	free(valor); valor = NULL;
}

void imprimirTexto(char* texto){
	printf("Solicitando imprimir texto.\n");
	char * txt = malloc(strlen(texto)+1);
	txt = strdup(texto);
	aplicar_protocolo_enviar(fdNucleo, IMPRIMIR_TEXTO, txt);
	free(txt);
}

void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo){

	printf("Entrada/Salida para el dispositivo: '%s' durante '%i' unidades de tiempo.\n",dispositivo,tiempo);
	pedidoIO * pedidoEntradaSalida = malloc(strlen(dispositivo)+ 5);
	pedidoEntradaSalida->nombreDispositivo = strdup((char*) dispositivo);
	pedidoEntradaSalida->tiempo = tiempo;

	aplicar_protocolo_enviar(fdNucleo,ENTRADA_SALIDA, pedidoEntradaSalida);

	free(pedidoEntradaSalida->nombreDispositivo); pedidoEntradaSalida->nombreDispositivo = NULL;
	free(pedidoEntradaSalida); pedidoEntradaSalida = NULL;
	devolvioPcb = POR_IO;
}

void s_wait(t_nombre_semaforo nombre_semaforo){

	char* id_semaforo = malloc(strlen(nombre_semaforo)+1);
	id_semaforo = strdup((char*) nombre_semaforo);

	aplicar_protocolo_enviar(fdNucleo, WAIT_REQUEST, id_semaforo);
	free(id_semaforo); id_semaforo = NULL;

	int head;
	aplicar_protocolo_recibir(fdNucleo, &head);

	if(head == WAIT_CON_BLOQUEO){
		// Mando la pcb bloqueada y la saco de ejecución:
		devolvioPcb = POR_WAIT;
		printf("Proceso bloqueado al hacer WAIT del semáforo: '%s'.\n", nombre_semaforo);
	}
	else{
		printf("Proceso continúa ejecutando luego de hacer WAIT del semáforo: '%s'.\n", nombre_semaforo);
	}
}

void s_signal(t_nombre_semaforo nombre_semaforo){

	char* id_semaforo = malloc(strlen(nombre_semaforo)+1);
	id_semaforo = strdup((char*) nombre_semaforo);

	aplicar_protocolo_enviar(fdNucleo, SIGNAL_REQUEST, id_semaforo);

	printf("SIGNAL del semáforo '%s'.\n", nombre_semaforo);
	free(id_semaforo); id_semaforo = NULL;
}

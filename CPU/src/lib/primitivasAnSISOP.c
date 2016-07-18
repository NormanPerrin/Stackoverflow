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
		printf("Definiendo nuevo argumento: '%c'.\n", var_nombre);
	}

		char * var_id = strdup(charAString(var_nombre));

		int var_pagina = pcbActual->primerPaginaStack;
		int var_offset = pcbActual->stackPointer;
		int var_size = INT;

		while(var_offset > tamanioPagina){
			(var_pagina)++;
			var_offset -= tamanioPagina;
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
				new_var->direccion.offset = var_offset;
				new_var->direccion.pagina = var_pagina;
				new_var->direccion.size = var_size;
				new_var->nombre = var_id;
				list_add(regStack->vars, new_var);
			}
			else{
				variable* new_arg = malloc(14);
				new_arg->direccion.offset = var_offset;
				new_arg->direccion.pagina = var_pagina;
				new_arg->direccion.size = var_size;
				new_arg->nombre = var_id;
				list_add(regStack->args, new_arg);
			}
			printf("'%c' -> Dirección lógica: %i, %i, %i.\n", var_nombre, var_pagina, var_offset, var_size);
			free (var_id); var_id = NULL;
			// Guardo el nuevo registro en el índice:
			list_add(pcbActual->indiceStack, regStack);
			// Valor a retornar:
			int var_stack_offset = pcbActual->stackPointer;
			// Actualizo parámetros del PCB:
			pcbActual->stackPointer += INT;
			int total_heap_offset = (pcbActual->paginas_codigo * tamanioPagina) + pcbActual->stackPointer;
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
		if(regStack->vars->elements_count > 0){ // si hay variables en la lista:

			// Obtengo la variable buscada:
			int i;
			for(i = 0; i<regStack->vars->elements_count; i++){

				variable* variable = list_get(regStack->vars, i);
				if(string_equals_ignore_case(variable->nombre, var_id)){

					free(var_id); var_id = NULL;

					int var_stack_page = variable->direccion.pagina - pcbActual->primerPaginaStack;
					int var_stack_offset = (var_stack_page*tamanioPagina) + variable->direccion.offset;

					return var_stack_offset;
				} // fin if equals
			} // fin for variables
		}
		printf("Error: No hay variables en el registro actual de stack.\n");
		return ERROR;
	} // Si entra acá es un argumento:
	else{
		if(regStack->args->elements_count > 0){

			// Obtengo el argumento buscado:
			int j;
			for(j = 0; j<regStack->args->elements_count; j++){

				variable* argumento = list_get(regStack->args, j);
				if(string_equals_ignore_case(argumento->nombre, var_id)){

					free(var_id); var_id = NULL;

					int arg_stack_page = argumento->direccion.pagina - pcbActual->primerPaginaStack;
					int arg_stack_offset = (arg_stack_page*tamanioPagina) + argumento->direccion.offset;

					return arg_stack_offset;
				} // fin if equals
			} // fin for argumentos
		}
		printf("Error: No hay argumentos en el registro actual de stack.\n");
		return ERROR;
	} // fin else argumentos
}

t_valor_variable dereferenciar(t_puntero total_heap_offset){

	printf("Dereferenciando variable...\n");
	solicitudLectura * var_direccion = malloc(sizeof(solicitudLectura));

	int num_pagina =  total_heap_offset / tamanioPagina;
	int offset = total_heap_offset % tamanioPagina;

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

		return ERROR;
	}
	else{ // no hubo error de lectura
		entrada = aplicar_protocolo_recibir(fdUMC, &head); // respuesta OK de UMC, recibo la variable leída
		if(head == DEVOLVER_VARIABLE){
			valor_variable = (int*)entrada;

			return *valor_variable;
		}
		else{
			printf("Error al leer variable del proceso actual.\n");
			exitPorErrorUMC();

			return ERROR;
		}
	} // fin else lectura ok
}

void asignar(t_puntero total_heap_offset, t_valor_variable valor){

	printf("Escribiendo variable...\n");
	solicitudEscritura * var_escritura = malloc(sizeof(solicitudEscritura));

	int num_pagina =  total_heap_offset / tamanioPagina;
	int offset = total_heap_offset % tamanioPagina;

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
	printf("La variable ha sido asignada.\n");
	return;
}

t_valor_variable obtenerValorCompartida(t_nombre_compartida var_compartida_nombre){

	printf("Obteniendo el valor de la variable compartida: '%s'.\n", var_compartida_nombre);
	char * variableCompartida = malloc(strlen(var_compartida_nombre)+1);
	void* entrada = NULL;
	int* valor_variable = NULL;
	int head;

	variableCompartida = (char*) var_compartida_nombre;

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

	variableCompartida->valor = var_compartida_valor;
	variableCompartida->nombre = (char*) var_compartida_nombre;

	aplicar_protocolo_enviar(fdNucleo, GRABAR_VAR_COMPARTIDA, variableCompartida);
	free(variableCompartida->nombre); variableCompartida->nombre = NULL;
	free(variableCompartida); variableCompartida = NULL;

	return var_compartida_valor;
}

void irAlLabel(t_nombre_etiqueta nombre_etiqueta){
	printf("Llendo a la etiqueta: '%s'...\n", nombre_etiqueta);
	t_puntero_instruccion num_instruccion = metadata_buscar_etiqueta(nombre_etiqueta, pcbActual->indiceEtiquetas, pcbActual->tamanioIndiceEtiquetas);
	pcbActual->pc = num_instruccion - 1;
	return;
}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){
	/* Reserva espacio para un nuevo contexto vacío donde guarda el contexto
	 *  de ejecución actual, para luego volver al mismo. */
	printf("Llamada con retorno. Preservando contexto de ejecución actual y posición de retorno.\n");

	// Calculo la dirección de retorno y la guardo:
	registroStack * nuevoRegistroStack = reg_stack_create();
	nuevoRegistroStack->retVar.pagina = donde_retornar / tamanioPagina;
	nuevoRegistroStack->retVar.offset = donde_retornar % tamanioPagina;
	nuevoRegistroStack->retVar.size = INT;

	nuevoRegistroStack->retPos = pcbActual->pc; // Guardo el valor actual del program counter
	list_add(pcbActual->indiceStack, nuevoRegistroStack);

	(pcbActual->indexActualStack)++;

	irAlLabel(etiqueta);
	return;
}

void retornar(t_valor_variable var_retorno){

	printf("Llamada a la función 'retornar'.\n");
	// Tomo contexto actual:
	int index = pcbActual->indexActualStack;
	registroStack* registroActual = list_get(pcbActual->indiceStack, index);

	// Limpio los argumentos del registro y descuento el espacio que ocupan en el stack en memoria:
	pcbActual->stackPointer -= (4* registroActual->args->elements_count);

	// Limpio las variables del registro y descuento el espacio que ocupan en el stack en memoria:
	pcbActual->stackPointer -= (4 * registroActual->vars->elements_count);

	// Calculo la dirección de retorno a partir de retVar:
	t_puntero offset_absoluto = (registroActual->retVar.pagina * tamanioPagina) + registroActual->retVar.offset;
	asignar(offset_absoluto, var_retorno);

	// Elimino el contexto actual del índice de stack:
	// Luego, seteo el contexto de ejecución actual en el index anterior:
	pcbActual->pc = registroActual->retPos;
	liberarRegistroStack(list_remove(pcbActual->indiceStack, pcbActual->indexActualStack));
	(pcbActual->indexActualStack)--;
	return;
}

void imprimir(t_valor_variable valor_mostrar){
	printf("Solicitando imprimir variable.\n");
	int * valor = malloc(INT);
	*valor = valor_mostrar;
	aplicar_protocolo_enviar(fdNucleo, IMPRIMIR, valor);
	free(valor); valor = NULL;
	return;
}

void imprimirTexto(char* texto){
	printf("Solicitando imprimir texto.\n");
	char * print_txt = malloc(strlen(texto)+1);
	print_txt = texto;
	aplicar_protocolo_enviar(fdNucleo, IMPRIMIR_TEXTO, print_txt);
	free(print_txt); print_txt = NULL;
	return;
}

void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo){

	printf("Entrada/Salida en dispositivo: '%s' durante '%i' unidades de tiempo.\n",dispositivo,tiempo);
	pedidoIO * pedidoEntradaSalida = malloc(strlen(dispositivo)+ 5);
	pedidoEntradaSalida->tiempo = tiempo;
	pedidoEntradaSalida->nombreDispositivo = strdup((char*) dispositivo);

	aplicar_protocolo_enviar(fdNucleo,ENTRADA_SALIDA, pedidoEntradaSalida);

	free(pedidoEntradaSalida->nombreDispositivo); pedidoEntradaSalida->nombreDispositivo = NULL;
	free(pedidoEntradaSalida); pedidoEntradaSalida = NULL;
	devolvioPcb = POR_IO;
	return;
}

void s_wait(t_nombre_semaforo nombre_semaforo){

	char* id_semaforo = malloc(strlen(nombre_semaforo)+1);
	id_semaforo = (char*) nombre_semaforo;

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
	return;
}

void s_signal(t_nombre_semaforo nombre_semaforo){

	char* id_semaforo = malloc(strlen(nombre_semaforo)+1);
	id_semaforo = (char*) nombre_semaforo;

	aplicar_protocolo_enviar(fdNucleo, SIGNAL_REQUEST, id_semaforo);

	printf("SIGNAL del semáforo '%s'.\n", nombre_semaforo);
	free(id_semaforo); id_semaforo = NULL;
	return;
}

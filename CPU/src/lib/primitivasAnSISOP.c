#include "primitivasAnSISOP.h"

/* FUNCIONES EXTRA */
bool esArgumento(t_nombre_variable identificador_variable){
	if(isdigit(identificador_variable)){
		return true;
	}else{
		return false;
	}
}

registroStack* reg_stack_create(){
	registroStack* reg = malloc(sizeof(registroStack));
	reg->cantidad_args = 0;
	reg->args = list_create();
	reg->cantidad_vars = 0;
	reg->vars = list_create();
	reg->retPos = 0;
	reg->retVar.offset = 0;
	reg->retVar.pagina = 0;
	reg->retVar.size = 0;

	return reg;
}

/* --- PRIMITIVAS --- */
t_puntero definirVariable(t_nombre_variable var_nombre){

	if(!esArgumento(var_nombre)){
		log_trace(logger, "Definiendo nueva variable: '%c'.", var_nombre);
	}
	else{
		log_trace(logger, "Definiendo nuevo argumento: '%c'.", var_nombre);
	}
		// Defino una nueva posición en el stack para la variable:
		int var_pagina = pcbActual->primerPaginaStack;
		int var_offset = pcbActual->stackPointer;

		while(var_offset > tamanioPagina){
			(var_pagina)++;
			var_offset -= tamanioPagina;
		}
		// Verifico si se desborda la pila en memoria:
		if(pcbActual->stackPointer + 4 > (tamanioPagina*tamanioStack)){
				log_trace(logger, "Stack Overflow al definir variable '%c'.", var_nombre);
				huboStackOverflow = true;

			return ERROR;
		}else{
			// Selecciono registro actual del índice de stack:
			registroStack* regStack = list_get(pcbActual->indiceStack, pcbActual->indiceStack->elements_count -1);

			if(regStack == NULL){ // si no hay registros, creo uno nuevo
				regStack = reg_stack_create();
				// Guardo el nuevo registro en el índice:
				list_add(pcbActual->indiceStack, regStack);
			}

			if(!esArgumento(var_nombre)){ // agrego nueva variable
				variable* new_var = malloc(sizeof(variable));
				new_var->nombre = var_nombre;
				new_var->direccion.pagina = var_pagina;
				new_var->direccion.offset = var_offset;
				new_var->direccion.size = INT;

				list_add(regStack->vars, new_var);
			}
			else{ // agrego nuevo argumento
				variable* new_arg = malloc(sizeof(variable));
				new_arg->nombre = var_nombre;
				new_arg->direccion.pagina = var_pagina;
				new_arg->direccion.offset = var_offset;
				new_arg->direccion.size = INT;

				list_add(regStack->args, new_arg);
			}

			log_trace(logger, "'%c' -> Dirección lógica definida: %i, %i, %i.", var_nombre, var_pagina, var_offset, INT);

			// Actualizo parámetros del PCB:
			int total_heap_offset = (pcbActual->paginas_codigo * tamanioPagina) + pcbActual->stackPointer;
			pcbActual->stackPointer += INT;
			pcbActual->paginaActualStack = (total_heap_offset + INT) / tamanioPagina;

			return total_heap_offset;
		} // fin else ERROR
}

t_puntero obtenerPosicionVariable(t_nombre_variable var_nombre){

	if(!esArgumento(var_nombre)){
		log_trace(logger, "Obteneniendo posición de la variable: '%c'.", var_nombre);
	}
	else{
		log_trace(logger, "Obteneniendo posición del argumento: '%c'.", var_nombre);
	}

	// Obtengo el registro del stack correspondiente al contexto de ejecución actual:
	registroStack* regStack = list_get(pcbActual->indiceStack, pcbActual->indiceStack->elements_count -1);

	// Busco en este registro la variable que coincida con el nombre solicitado:
	if(!esArgumento(var_nombre)){ // Si entra acá es una variable:
		if(regStack->vars->elements_count > 0){ // si hay variables en la lista:

			// Obtengo la variable buscada:
			int i;
			for(i = 0; i<regStack->vars->elements_count; i++){

				variable* variable = list_get(regStack->vars, i);
				if(variable->nombre == var_nombre){

					int var_offset_absoluto = (variable->direccion.pagina * tamanioPagina) + variable->direccion.offset;

					return var_offset_absoluto;
				} // fin if equals
			} // fin for variables
		} // fin if hay variables
		log_error(logger, "No hay variables en el registro actual de stack.");
		return ERROR;
	} // Si entra acá es un argumento:
	else{
		if(regStack->args->elements_count > 0){

			// Obtengo el argumento buscado:
			int j;
			for(j = 0; j<regStack->args->elements_count; j++){

				variable* argumento = list_get(regStack->args, j);
				if(argumento->nombre == var_nombre){

					int arg_offset_absoluto = (argumento->direccion.pagina * tamanioPagina) + argumento->direccion.offset;

					return arg_offset_absoluto;
				} // fin if equals
			} // fin for argumentos
		} // fin if hay argumentos
		log_error(logger, "No hay argumentos en el registro actual de stack.");
		return ERROR;
	} // fin else argumentos
}

t_valor_variable dereferenciar(t_puntero total_heap_offset){

	solicitudLectura * var_direccion = malloc(sizeof(solicitudLectura));

	var_direccion->pagina = total_heap_offset / tamanioPagina;
	var_direccion->offset = total_heap_offset % tamanioPagina;
	var_direccion->tamanio = INT;

	log_trace(logger, "Solicitud Lectura -> Página: %i, Offset: %i, Size: %i.",
			var_direccion->pagina, var_direccion->offset, INT);
	aplicar_protocolo_enviar(fdUMC, PEDIDO_LECTURA_VARIABLE, var_direccion);
	free(var_direccion); var_direccion = NULL;

	// Valido el pedido de lectura a UMC:
	if(!recibirYvalidarEstadoDelPedidoAUMC()){ // hubo error de lectura
		log_error(logger, "La variable no pudo dereferenciarse.");
		exitPorErrorUMC();

		return ERROR;
	}
	else{ // no hubo error de lectura
		int head;
		void* entrada = NULL;
		entrada = aplicar_protocolo_recibir(fdUMC, &head);  // respuesta OK de UMC, recibo la variable leída

		if(entrada == NULL){
			log_error(logger, "UMC se ha desconectado.");
			exitFailureCPU();
		}

		if(head == DEVOLVER_VARIABLE){
			//int valor = atoi((char*) entrada);

			int valor = 0;
			char *aux = malloc(INT);
			memcpy(aux, entrada, INT);
			valor = (int)*(aux);
			log_trace(logger, "Variable dereferenciada -> Valor: %d.", valor);
			free(entrada); entrada = NULL;

			return valor;
		}
		else{
			log_error(logger, "La variable no pudo dereferenciarse.");
			exitPorErrorUMC();

			return ERROR;
		}
	} // fin else lectura ok
}

void asignar(t_puntero total_heap_offset, t_valor_variable valor){

	solicitudEscritura* var_escritura = malloc(sizeof(solicitudEscritura));

		var_escritura->pagina = total_heap_offset / tamanioPagina;
		var_escritura->offset = total_heap_offset % tamanioPagina;
		/*var_escritura->contenido = malloc(INT);
		sprintf(var_escritura->contenido, "%d", valor);

	// Relleno con barra cero los espacios vacíos:
		int chars_numericos = strlen(var_escritura->contenido);
		int i;
		for(i = chars_numericos+1; i <= INT-1; i++){
			var_escritura->contenido[i] = '\0';
		}*/
		var_escritura->contenido = (char*) &valor;

	log_trace(logger, "Solicitud Escritura -> Página: %i, Offset: %i, Contenido: %d.",
			var_escritura->pagina, var_escritura->offset, valor);

	aplicar_protocolo_enviar(fdUMC, PEDIDO_ESCRITURA, var_escritura);
	//free(var_escritura->contenido); var_escritura->contenido = NULL;
	free(var_escritura); var_escritura = NULL;

	// Valido el pedido de lectura a UMC:
	if(!recibirYvalidarEstadoDelPedidoAUMC()){
		log_error(logger, "La variable no pudo asignarse en memoria.");
		exitPorErrorUMC();
	}
}

t_valor_variable obtenerValorCompartida(t_nombre_compartida var_compartida_nombre){

	log_trace(logger, "Obteniendo valor de Variable Compartida: '%s'.", var_compartida_nombre);
	aplicar_protocolo_enviar(fdNucleo, OBTENER_VAR_COMPARTIDA, var_compartida_nombre);

	void* entrada = NULL;
	int head;
	entrada = aplicar_protocolo_recibir(fdNucleo, &head);

	if(entrada == NULL){
		log_error(logger, "Núcleo se ha desconectado.");
		exitFailureCPU();
	}

	if(head == DEVOLVER_VAR_COMPARTIDA){
		t_valor_variable valor = *((int*) entrada);
		log_trace(logger, "Variable Compartida: '%s' -> Valor: '%d'.", var_compartida_nombre, valor);

		return valor;
	}
	else{
		log_error(logger, "No se pudo obtener valor de Variable Compartida '%s'.", var_compartida_nombre);

		return ERROR;
	}
}

t_valor_variable asignarValorCompartida(t_nombre_compartida var_compartida_nombre, t_valor_variable var_compartida_valor){

	log_trace(logger, "Asignando el valor '%d' a Variable Compartida '%s'.", var_compartida_valor, var_compartida_nombre);
	var_compartida * variableCompartida = malloc(strlen(var_compartida_nombre)+ 5);

	variableCompartida->valor = var_compartida_valor;
	variableCompartida->nombre = var_compartida_nombre;

	aplicar_protocolo_enviar(fdNucleo, GRABAR_VAR_COMPARTIDA, variableCompartida);

	return var_compartida_valor;
}

void irAlLabel(t_nombre_etiqueta nombre_etiqueta){

	log_trace(logger, "Llendo a la etiqueta: '%s'.", nombre_etiqueta);
	t_puntero_instruccion posicion_etiqueta = metadata_buscar_etiqueta(nombre_etiqueta, pcbActual->indiceEtiquetas, pcbActual->tamanioIndiceEtiquetas);

	if(posicion_etiqueta == ERROR) log_error(logger, "La etiqueta '%s' no se encuentra en el índice.", nombre_etiqueta);

	pcbActual->pc = posicion_etiqueta - 1;
}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){
	/* Reserva espacio para un nuevo contexto vacío para la función que se llama,
	 * y se guarda iformación sobre el contexto de ejecución actual. */
	log_trace(logger, "Llamada con retorno. Preservando contexto de ejecución actual.");

	// Calculo la dirección de retorno y la guardo:
	registroStack * nuevoRegistro = reg_stack_create();
	nuevoRegistro->retVar.pagina = donde_retornar / tamanioPagina;
	nuevoRegistro->retVar.offset = donde_retornar % tamanioPagina;
	nuevoRegistro->retVar.size = INT;

	nuevoRegistro->retPos = pcbActual->pc; // Guardo el valor actual del program counter
	list_add(pcbActual->indiceStack, nuevoRegistro);

	irAlLabel(etiqueta);
}

void retornar(t_valor_variable var_retorno){

	log_trace(logger, "Llamada a función 'retornar'.");
	// Tomo contexto actual:
	registroStack* registroActual = list_get(pcbActual->indiceStack, pcbActual->indiceStack->elements_count -1);
	// Calculo la dirección de retorno a partir de retVar:
	t_puntero offset_absoluto = (registroActual->retVar.pagina * tamanioPagina) + registroActual->retVar.offset;
	asignar(offset_absoluto, var_retorno);

	finalizar();
}

void imprimir(t_valor_variable valor){

	log_trace(logger, "Solicitando imprimir variable.");
	aplicar_protocolo_enviar(fdNucleo, IMPRIMIR, &valor);
}

void imprimirTexto(char* texto){

	log_trace(logger, "Solicitando imprimir texto.");
	texto = _string_trim(texto);
	aplicar_protocolo_enviar(fdNucleo, IMPRIMIR_TEXTO, texto);
}

void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo){

	log_trace(logger, "Entrada/Salida en dispositivo: '%s' durante '%i' unidades de tiempo.", dispositivo, tiempo);
	pedidoIO * pedidoEntradaSalida = malloc(strlen(dispositivo)+ 5);
	pedidoEntradaSalida->tiempo = tiempo;
	pedidoEntradaSalida->nombreDispositivo = dispositivo;

	aplicar_protocolo_enviar(fdNucleo, ENTRADA_SALIDA, pedidoEntradaSalida);
	free(pedidoEntradaSalida); pedidoEntradaSalida = NULL;

	devolvioPcb = POR_IO;
}

void s_wait(t_nombre_semaforo nombre_semaforo){

	aplicar_protocolo_enviar(fdNucleo, WAIT_REQUEST, nombre_semaforo);

	int head;
	void* entrada = NULL;
	entrada = aplicar_protocolo_recibir(fdNucleo, &head);

	if(head == RESPUESTA_WAIT && entrada != NULL){

		int respuesta = *((int*) entrada);
		if(respuesta == CON_BLOQUEO){
			// Mando la pcb bloqueada y la saco de ejecución:
			devolvioPcb = POR_WAIT;
			log_trace(logger, "Proceso #%d bloqueado al hacer WAIT del semáforo: '%s'.", pcbActual->pid, nombre_semaforo);
		}
		else{
			log_trace(logger, "WAIT del semáforo: '%s'. No hubo bloqueo.", pcbActual->pid, nombre_semaforo);
		}
	}
	else{
		log_error(logger, "Núcleo se ha desconectado.");
		exitFailureCPU();
	}
}

void s_signal(t_nombre_semaforo nombre_semaforo){

	aplicar_protocolo_enviar(fdNucleo, SIGNAL_REQUEST, nombre_semaforo);
	log_trace(logger, "SIGNAL del semáforo '%s'.", nombre_semaforo);
}

void llamarSinRetorno(t_nombre_etiqueta etiqueta){

	log_trace(logger, "Llamada sin retorno.");
    registroStack* nuevoRegistro = reg_stack_create();
    nuevoRegistro->retPos = pcbActual->pc; // Guardo el valor actual del program counter
    list_add(pcbActual->indiceStack, nuevoRegistro);

    irAlLabel(etiqueta);
}

void restaurarContextoDeEjecucion(){

	log_trace(logger, "Restaurando contexto de ejecución anterior.");
	registroStack* registroActual = list_remove(pcbActual->indiceStack, pcbActual->indiceStack->elements_count -1);

	// Limpio los argumentos del registro y descuento el espacio que ocupan en el stack en memoria:
	int i;
	for(i = 0; i < registroActual->args->elements_count; i++){
		variable* arg = list_remove(registroActual->args, i);
		pcbActual->stackPointer -= INT;
		free(arg); arg = NULL;
	}
	// Limpio las variables del registro y descuento el espacio que ocupan en el stack en memoria:
	for(i = 0; i < registroActual->vars->elements_count; i++){
		variable* var = list_remove(registroActual->vars, i);
		pcbActual->stackPointer -= INT;
		free(var); var = NULL;
	}
	// Elimino el contexto actual del índice de stack, y seteo el nuevo contexto de ejecución en el index anterior:
	pcbActual->pc = registroActual->retPos;
	//liberarRegistroStack(registroActual);
	free(registroActual); registroActual = NULL;
}

void finalizar(void){

    restaurarContextoDeEjecucion();

    if(list_is_empty(pcbActual->indiceStack)){
    	log_trace(logger, "Finalizar contexto principal.");
    	finalizoPrograma = true;
    }
    else{
    	log_trace(logger, "Finalizar contexto actual.");
    }
}

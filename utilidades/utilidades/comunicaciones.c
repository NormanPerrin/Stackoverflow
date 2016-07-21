#include "comunicaciones.h"

// SEREALIZAR: Del mensaje listo para enviar, al buffer
// DESEREAILZAR: Del buffer, al mensaje listo para recibir

void aplicar_protocolo_enviar(int fdReceptor, int head, void *mensaje){

	int desplazamiento = 0, tamanioMensaje, tamanioTotalAEnviar;

	if (head < 1 || head > FIN_DEL_PROTOCOLO) printf("Error al enviar mensaje.\n");
	// Calculo el tamaño del mensaje:
	tamanioMensaje = calcularTamanioMensaje(head, mensaje);

	// Serealizo el mensaje según el protocolo (me devuelve el mensaje empaquetado):
	void *mensajeSerealizado = serealizar(head, mensaje, tamanioMensaje);

	// Lo que se envía es: head + tamaño del msj + el msj serializado:
	tamanioTotalAEnviar = 2*INT + tamanioMensaje;

	// Meto en el buffer las tres cosas:
	void *buffer = malloc(tamanioTotalAEnviar);
	memcpy(buffer + desplazamiento, &head, INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &tamanioMensaje, INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, mensajeSerealizado, tamanioMensaje);

	// Envío la totalidad del paquete (lo contenido en el buffer):
	enviarPorSocket(fdReceptor, buffer, tamanioTotalAEnviar);

	free(mensajeSerealizado);
	free(buffer);
}

void * aplicar_protocolo_recibir(int fdEmisor, int* head){

	// Validar contra NULL al recibir en cada módulo.
	// Recibo primero el head:
	int recibido = recibirPorSocket(fdEmisor, head, INT);

	if (*head < 1 || *head > FIN_DEL_PROTOCOLO || recibido <= 0){ // DESCONEXIÓN
		//printf("Error al recibir mensaje.\n");
		return NULL;
	}

	// Recibo ahora el tamaño del mensaje:
	int* tamanioMensaje = malloc(INT);
	recibido = recibirPorSocket(fdEmisor, tamanioMensaje, INT);
		if (recibido <= 0) return NULL;
	// Recibo por último el mensaje serealizado:
	void* mensaje = malloc(*tamanioMensaje);
	recibido = recibirPorSocket(fdEmisor, mensaje, *tamanioMensaje);
		if (recibido <= 0) return NULL;
	// Deserealizo el mensaje según el protocolo:
	void* buffer = deserealizar(*head, mensaje, *tamanioMensaje);

	free(tamanioMensaje);
	free(mensaje);

	return buffer;
} // Se debe castear el mensaje al recibirse (indicar el tipo de dato que debe matchear con el void*)

int calcularTamanioMensaje(int head, void* mensaje){

	int tamanio;

	switch(head){
		// CASE 0: El mensaje es un texto (string)
			case ENVIAR_SCRIPT:{
				string* script = (string*) mensaje;
				tamanio = script->tamanio + INT;
				break;
			}
		// CASE 1: El mensaje es un texto (char*)
			case IMPRIMIR_TEXTO: case DEVOLVER_INSTRUCCION: case WAIT_REQUEST: case SIGNAL_REQUEST:
			case OBTENER_VAR_COMPARTIDA: case DEVOLVER_PAGINA:{
				tamanio = strlen((char*)mensaje)+ 1;
				break;
			}
		// CASE 2: El mensaje es un texto (char*) más un valor entero (int)
			case ENTRADA_SALIDA: case GRABAR_VAR_COMPARTIDA:{
				pedidoIO* msj = (pedidoIO*)mensaje;
				tamanio = strlen(msj->nombreDispositivo)+ 1 + INT;
				break;
			}
		// CASE 3: El mensaje es un texto (char*) más dos valores enteros (int)
			case ESCRIBIR_PAGINA:{
				solicitudEscribirPagina* msj = (solicitudEscribirPagina*) mensaje;
				tamanio = strlen(msj->contenido) + 1 + 2*INT;
				break;
			}
		// CASE 4: El mensaje es un PCB (pcb)
			case PCB: case PCB_FIN_EJECUCION: case PCB_FIN_QUANTUM: case PCB_ENTRADA_SALIDA:
			case PCB_WAIT:{
				tamanio = calcularTamanioPcb((pcb*) mensaje);
				break;
			}
		// CASE 5: El mensaje es un valor entero (int)
			case DEVOLVER_VARIABLE: case RESPUESTA_PEDIDO: case FINALIZAR_PROGRAMA: case IMPRIMIR:
			case PROGRAMA_NEW: case ABORTO_PROCESO: case INDICAR_PID: case DEVOLVER_VAR_COMPARTIDA:
			case TAMANIO_STACK: case SENIAL_SIGUSR1:{
				tamanio = INT;
				break;
				}
		// CASE 6: El mensaje son dos valores enteros (int)
			case LEER_PAGINA:{
				tamanio = 2*INT;
				break;
			}
		// CASE 7: El mensaje son tres valores enteros (int)
			case PEDIDO_LECTURA_VARIABLE: case PEDIDO_ESCRITURA: case PEDIDO_LECTURA_INSTRUCCION:{
				tamanio = 3*INT;
				break;
			}
		// CASE 8: El mensaje es un texto (string) más dos valores enteros (int)
			case INICIAR_PROGRAMA:{
				inicioPrograma* msj = (inicioPrograma*) mensaje;
				tamanio = msj->contenido.tamanio + 3*INT;
				break;
			}
		} // fin switch head
	return tamanio;
}

void * serealizar(int head, void * mensaje, int tamanio){

	void * buffer;

	switch(head){
	// CASE 0: El mensaje es un texto (string)
	case ENVIAR_SCRIPT:{
		buffer = serealizarString(mensaje, tamanio);
		break;
	}
	// CASE 1: El mensaje es un texto (char*)
	case IMPRIMIR_TEXTO: case DEVOLVER_INSTRUCCION: case WAIT_REQUEST: case SIGNAL_REQUEST:
	case OBTENER_VAR_COMPARTIDA: case DEVOLVER_PAGINA:{
			buffer = malloc(tamanio);
			memcpy(buffer, mensaje, tamanio);
			break;
		}
	// CASE 2: El mensaje es un texto (char*) más un valor entero (int)
	case ENTRADA_SALIDA: case GRABAR_VAR_COMPARTIDA:{
			buffer = serealizarTextoMasUnInt(mensaje, tamanio);
			break;
		}
	// CASE 3: El mensaje es un texto (char*) más dos valores enteros (int)
	case ESCRIBIR_PAGINA:{
		buffer = serealizarPedidoEscrituraPagina(mensaje, tamanio);
			break;
		}
	// CASE 4: El mensaje es un PCB (pcb)
	case PCB: case PCB_FIN_EJECUCION: case PCB_FIN_QUANTUM: case PCB_ENTRADA_SALIDA: case PCB_WAIT:{
		buffer = serealizarPcb(mensaje, tamanio);
			break;
		}
	// CASE 5: El mensaje es un valor entero (int)
	case DEVOLVER_VARIABLE: case RESPUESTA_PEDIDO: case FINALIZAR_PROGRAMA: case IMPRIMIR: case SENIAL_SIGUSR1:
	case PROGRAMA_NEW: case ABORTO_PROCESO: case INDICAR_PID: case DEVOLVER_VAR_COMPARTIDA:
	case WAIT_SIN_BLOQUEO: case WAIT_CON_BLOQUEO: case TAMANIO_STACK:{
		buffer = malloc(tamanio);
		memcpy(buffer, mensaje, tamanio);
			break;
		}
	// CASE 6: El mensaje son dos valores enteros (int)
	case LEER_PAGINA:{
		buffer = serealizarDosInt(mensaje, tamanio);
			break;
		}
	// CASE 7: El mensaje son tres valores enteros (int)
	case PEDIDO_LECTURA_VARIABLE: case PEDIDO_LECTURA_INSTRUCCION: case PEDIDO_ESCRITURA:{
		buffer = serealizarTresInt(mensaje, tamanio);
			break;
		}
	// CASE 8: El mensaje es un texto (string) más dos valores enteros (int)
	case INICIAR_PROGRAMA:{
		buffer = serealizarInicioPrograma(mensaje, tamanio);
			break;
		}
	} // fin switch head
	return buffer;
}

void * deserealizar(int head, void * buffer, int tamanio){

	void * mensaje;

	switch(head){
		// CASE 0: El mensaje es un texto (string)
		case ENVIAR_SCRIPT:{
			mensaje = deserealizarString(buffer, tamanio);
			break;
		}
		// CASE 1: El mensaje es un texto (char*)
		case IMPRIMIR_TEXTO: case DEVOLVER_INSTRUCCION: case WAIT_REQUEST: case SIGNAL_REQUEST:
		case OBTENER_VAR_COMPARTIDA: case DEVOLVER_PAGINA:{
			char* msj = malloc(tamanio);
			memcpy(msj, buffer, tamanio);
			mensaje = msj;
				break;
			}
		// CASE 2: El mensaje es un texto (char*) más un valor entero (int)
		case ENTRADA_SALIDA:{
			mensaje = deserealizarTextoMasUnInt(buffer, tamanio);
				break;
			}
		case GRABAR_VAR_COMPARTIDA:{
			mensaje = (var_compartida*)deserealizarTextoMasUnInt(buffer, tamanio);
				break;
		}
		// CASE 3: El mensaje es un texto (char*) más dos valores enteros (int)
		case ESCRIBIR_PAGINA:{
			mensaje = deserealizarPedidoEscrituraPagina(buffer, tamanio);
				break;
			}
		// CASE 4: El mensaje es un pcb (pcb)
		case PCB: case PCB_FIN_EJECUCION: case PCB_FIN_QUANTUM: case PCB_ENTRADA_SALIDA: case PCB_WAIT:{
			mensaje = deserealizarPcb(buffer, tamanio);
				break;
			}
		// CASE 5: El mensaje es un valor entero (int)
		case DEVOLVER_VARIABLE: case RESPUESTA_PEDIDO: case FINALIZAR_PROGRAMA: case IMPRIMIR:
		case PROGRAMA_NEW: case ABORTO_PROCESO: case INDICAR_PID: case DEVOLVER_VAR_COMPARTIDA:
		case WAIT_SIN_BLOQUEO: case WAIT_CON_BLOQUEO: case SENIAL_SIGUSR1: case TAMANIO_STACK:{
			int* msj = malloc(tamanio);
			memcpy(msj, buffer, tamanio);
			mensaje = msj;
				break;
			}
		// CASE 6: El mensaje son dos valores enteros (int)
		case LEER_PAGINA:{
			mensaje = deserealizarDosInt(buffer, tamanio);
				break;
			}
		// CASE 7: El mensaje son tres valores enteros (int)
		case PEDIDO_LECTURA_VARIABLE: case PEDIDO_LECTURA_INSTRUCCION:{
			mensaje = (solicitudLectura*)deserealizarTresInt(buffer, tamanio);
				break;
			}
		case PEDIDO_ESCRITURA:{
			mensaje = (solicitudEscritura*)deserealizarTresInt(buffer, tamanio);
			break;
		}
		// CASE 8: El mensaje es un texto (string) más dos valores enteros (int)
		case INICIAR_PROGRAMA:{
			mensaje = deserealizarInicioPrograma(buffer, tamanio);
				break;
			}
		}
	return mensaje;
} // Se debe castear lo retornado (indicar el tipo de dato que debe matchear con el void*)

/**** SEREALIZACIONES PARTICULARES ****/
void* serealizarString(void* mensaje, int tamanio){
	string* msj = (string*) mensaje;
	int desplazamiento = 0;

	void * buffer = malloc(tamanio);
	memcpy(buffer + desplazamiento, &(msj->tamanio), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, msj->cadena, msj->tamanio);

	return buffer;
}

string* deserealizarString(void* buffer, int tamanio){
	int desplazamiento = 0;

	string * msj = malloc(sizeof(string));
	memcpy(&msj->tamanio, buffer + desplazamiento, INT);
		desplazamiento += INT;
	msj->cadena = malloc(msj->tamanio);
	memcpy(msj->cadena, buffer + desplazamiento, msj->tamanio);

	return msj;
}

void* serealizarInicioPrograma(void* mensaje, int tamanio){
	inicioPrograma* msj = (inicioPrograma*) mensaje;
	int desplazamiento = 0;

	void * buffer = malloc(tamanio);
	memcpy(buffer + desplazamiento, &(msj->pid), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(msj->paginas), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(msj->contenido.tamanio), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, msj->contenido.cadena, msj->contenido.tamanio);

	return buffer;
}

inicioPrograma* deserealizarInicioPrograma(void* buffer, int tamanio){
	int desplazamiento = 0;

	inicioPrograma* msj = malloc(sizeof(inicioPrograma));
	memcpy(&msj->pid, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&msj->paginas, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&msj->contenido.tamanio, buffer + desplazamiento, INT);
		desplazamiento += INT;
	msj->contenido.cadena = malloc(msj->contenido.tamanio);
	memcpy(msj->contenido.cadena, buffer + desplazamiento, msj->contenido.tamanio);

	return msj;
}

void* serealizarTextoMasUnInt(void* mensaje, int tamanio){

		pedidoIO* msj = (pedidoIO*) mensaje;
		int desplazamiento = 0;
		int string_size = tamanio-4;

		void * buffer = malloc(tamanio);
		memcpy(buffer + desplazamiento, &(msj->tiempo), INT);
			desplazamiento += INT;
		memcpy(buffer + desplazamiento, msj->nombreDispositivo, string_size);

		return buffer;
}

pedidoIO* deserealizarTextoMasUnInt(void* buffer, int tamanio){

	int desplazamiento = 0;
	int string_size = tamanio-4;

		pedidoIO * msj = malloc(sizeof(pedidoIO));
		memcpy(&msj->tiempo, buffer + desplazamiento, INT);
			desplazamiento += INT;
		msj->nombreDispositivo = malloc(string_size);
		memcpy(msj->nombreDispositivo, buffer + desplazamiento, string_size);

		return msj;
}

void*  serealizarPedidoEscrituraPagina(void* mensaje, int tamanio){

	solicitudEscribirPagina* msj = (solicitudEscribirPagina*) mensaje;
	int desplazamiento = 0;
	int contenido_size = tamanio-8;

		void * buffer = malloc(tamanio);
		memcpy(buffer + desplazamiento, &(msj->pid), INT);
			desplazamiento += INT;
		memcpy(buffer + desplazamiento, &(msj->pagina), INT);
			desplazamiento += INT;
		memcpy(buffer + desplazamiento, msj->contenido, contenido_size);

		return buffer;
}

solicitudEscribirPagina*  deserealizarPedidoEscrituraPagina(void* buffer, int tamanio){

	int desplazamiento = 0;
	int contenido_size = tamanio-8;

	solicitudEscribirPagina *msj = malloc(sizeof(solicitudEscribirPagina));
	memcpy(&msj->pid, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&msj->pagina, buffer + desplazamiento, INT);
		desplazamiento += INT;
	msj->contenido = malloc(contenido_size);
	memcpy(msj->contenido, buffer + desplazamiento, contenido_size);

	return msj;
}

void* serealizarDosInt(void* mensaje, int tamanio){

	solicitudLeerPagina* msj = (solicitudLeerPagina*) mensaje;
	int desplazamiento = 0;

	void * buffer = malloc(tamanio);
	memcpy(buffer + desplazamiento, &(msj->pid), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(msj->pagina), INT);

		return buffer;
}

solicitudLeerPagina* deserealizarDosInt(void* buffer, int tamanio){

	int desplazamiento = 0;

	solicitudLeerPagina* msj = malloc(sizeof(solicitudLeerPagina));
	memcpy(&msj->pid, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&msj->pagina, buffer + desplazamiento, INT);

		return msj;
}

void* serealizarTresInt(void* mensaje, int tamanio){

	direccion* msj = (direccion*) mensaje;
	int desplazamiento = 0;

	void * buffer = malloc(tamanio);
	memcpy(buffer + desplazamiento, &(msj->pagina), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(msj->offset), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(msj->size), INT);

		return buffer;
}

direccion* deserealizarTresInt(void* buffer, int tamanio){

	int desplazamiento = 0;

	direccion * msj = malloc(DIRECCION);
	memcpy(&msj->pagina, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&msj->offset, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&msj->size, buffer + desplazamiento, INT);

		return msj;
}

int calcularTamanioIndiceStack(pcb* unPcb){
	int tamanio_stack = 0;

	void calcularTamanioRegistroStack(registroStack* reg){
		int tamanio_fijo_registro = INT + // Cantidad variables, campo lista
									INT + // Cantidad argumentos, campo lista
									INT + // retPos
									INT + // Cantidad variables
									INT + // Cantidad argumentos
									DIRECCION; // retVar
		// Tamaño variables y argumentos:
		int tamanio_args_y_vars = (reg->args->elements_count * VARIABLE) + (reg->vars->elements_count * VARIABLE);

		int tamanio_registro = tamanio_fijo_registro + tamanio_args_y_vars;

		tamanio_stack += tamanio_registro;
	}

	list_iterate(unPcb->indiceStack, (void*) calcularTamanioRegistroStack);

	return tamanio_stack + INT; // Sumo además 4 bytes de elements_count del stack
}

int sizeof_instruccion(t_intructions *instrucciones){
	int sizeof_start_instruccion = sizeof(instrucciones->start);
	int sizeof_offset = sizeof(instrucciones->offset);

	return (sizeof_start_instruccion + sizeof_offset);
}

int calcularTamanioPcb(pcb* unPcb){
	int tamanio;
	int code_index_size = sizeof_instruccion(unPcb->indiceCodigo) * (unPcb->cantidad_instrucciones);
	int stack_index_size = calcularTamanioIndiceStack(unPcb);
	// Sumo 56 bytes por los 14 int que tiene + los tamaños de los tres índices:
	// Tamaños índice etiquetas y código vienen de la creación del PCB.
	tamanio = 56  + code_index_size + stack_index_size + unPcb->tamanioIndiceEtiquetas;

	return tamanio;
}

int getStartInstruccion(t_intructions instruccion){ return instruccion.start; }

int getOffsetInstruccion (t_intructions instruccion){ return instruccion.offset; }

t_intructions cargarIndiceCodigo(t_puntero_instruccion comienzo_instruccion, t_size longitud_instruccion){

	t_intructions instruccion = { .start = comienzo_instruccion, .offset = longitud_instruccion };

	return instruccion;
}

void * serealizarPcb(void * mensaje, int tamanio){

	pcb* unPcb = (pcb*) mensaje;
	int desplazamiento = 0;

	// Copio los campos enteros:
	void * buffer = malloc(tamanio);
	memcpy(buffer + desplazamiento, &(unPcb->cantidad_instrucciones), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPcb->id_cpu), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPcb->paginaActualCodigo), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPcb->paginaActualStack), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPcb->paginas_codigo), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPcb->paginas_stack), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPcb->pc), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPcb->pid), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPcb->primerPaginaStack), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPcb->quantum), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPcb->quantum_sleep), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPcb->stackPointer), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPcb->tamanioIndiceEtiquetas), INT);
		desplazamiento += INT;

		unPcb->cantidad_registros_stack = unPcb->indiceStack->elements_count;
	memcpy(buffer + desplazamiento, &(unPcb->cantidad_registros_stack), INT);
		desplazamiento += INT;

		// Copio el índice de código:
	int contador_instrucciones = 0;

	while (contador_instrucciones < unPcb->cantidad_instrucciones){

		t_puntero_instruccion start_instruccion = getStartInstruccion((unPcb->indiceCodigo)[contador_instrucciones]);
		memcpy(buffer + desplazamiento, &start_instruccion, sizeof(t_puntero_instruccion));
			desplazamiento += sizeof(t_puntero_instruccion);

		t_size offset_instruccion = getOffsetInstruccion((unPcb->indiceCodigo)[contador_instrucciones]);
		memcpy(buffer + desplazamiento, &offset_instruccion, sizeof(t_size));
			desplazamiento += sizeof(t_size);

		contador_instrucciones++;
	} // fin carga índice de código

		// Copio el índice de stack:

	void serealizarIndiceStack(registroStack* reg){

		void serealizarListaVariables(variable* var){
			memcpy(buffer + desplazamiento, &var->nombre, CHAR);
				desplazamiento +=  CHAR;
			memcpy(buffer + desplazamiento, &var->direccion, DIRECCION);
				desplazamiento += DIRECCION;
		}

		reg->cantidad_args = reg->args->elements_count;
		memcpy(buffer + desplazamiento, &(reg->cantidad_args), INT);
			desplazamiento += INT;

		list_iterate(reg->args, (void*) serealizarListaVariables);

		memcpy(buffer + desplazamiento, &(reg->args->elements_count), INT);
			desplazamiento += INT;

		reg->cantidad_vars = reg->vars->elements_count;
		memcpy(buffer + desplazamiento, &(reg->cantidad_vars), INT);
			desplazamiento += INT;

		list_iterate(reg->vars, (void*) serealizarListaVariables);

		memcpy(buffer + desplazamiento, &(reg->vars->elements_count), 4);
			desplazamiento += 4;

		memcpy(buffer + desplazamiento, &reg->retPos, INT);
			desplazamiento += INT;
		memcpy(buffer + desplazamiento, &reg->retVar, DIRECCION);
			desplazamiento += DIRECCION;
	} // fin función serealizadora índice de stack

	list_iterate(unPcb->indiceStack, (void*) serealizarIndiceStack);

	// Copio el int 'elements_count' campo del índice de stack:
	memcpy(buffer + desplazamiento, &(unPcb->indiceStack->elements_count), INT);
		desplazamiento += INT;
	// fin carga índice de stack

	// Copio el índice de etiquetas:
	memcpy(buffer + desplazamiento, unPcb->indiceEtiquetas, unPcb->tamanioIndiceEtiquetas);

	return buffer;
}

pcb * deserealizarPcb(void * buffer, int tamanio){

	int desplazamiento = 0;
	pcb * unPcb = malloc(sizeof(pcb));

	// Copio los valores enteros:
	memcpy(&unPcb->cantidad_instrucciones, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&unPcb->id_cpu, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&unPcb->paginaActualCodigo, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&unPcb->paginaActualStack, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&unPcb->paginas_codigo, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&unPcb->paginas_stack, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&unPcb->pc, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&unPcb->pid, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&unPcb->primerPaginaStack, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&unPcb->quantum, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&unPcb->quantum_sleep, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&unPcb->stackPointer, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&unPcb->tamanioIndiceEtiquetas, buffer + desplazamiento, INT);
		desplazamiento += INT;

	memcpy(&unPcb->cantidad_registros_stack, buffer + desplazamiento, INT);
		desplazamiento += INT;

		// Copio índice de código:
	int contador_instrucciones = 0;

	unPcb->indiceCodigo = malloc(sizeof(t_intructions)*(unPcb->cantidad_instrucciones));

	while(contador_instrucciones < unPcb->cantidad_instrucciones){

		t_puntero_instruccion start_instruccion = 0;
		t_size offset_instruccion = 0;

		memcpy(&start_instruccion, buffer + desplazamiento, sizeof(t_puntero_instruccion));
			desplazamiento += sizeof(t_puntero_instruccion);

		memcpy(&offset_instruccion, buffer + desplazamiento, sizeof(t_size));
			desplazamiento += sizeof(t_size);

		(unPcb->indiceCodigo)[contador_instrucciones] = cargarIndiceCodigo(start_instruccion, offset_instruccion);

		contador_instrucciones++;
	} // fin carga índice código

		// Copio el índice de stack:
	unPcb->indiceStack = list_create();

	int contador_registros_stack = 0;

	while(contador_registros_stack < unPcb->cantidad_registros_stack){

		registroStack *reg = malloc(sizeof(registroStack));

		memcpy(&reg->cantidad_args, buffer + desplazamiento, INT);
			desplazamiento += INT;

		reg->args = list_create();

		int contador_argumentos = 0;

		while(contador_argumentos < reg->cantidad_args){

			variable *arg = malloc(sizeof(variable));

			memcpy(&arg->nombre, buffer + desplazamiento, CHAR);
				desplazamiento += CHAR;
			memcpy(&arg->direccion, buffer + desplazamiento, DIRECCION);
				desplazamiento += DIRECCION;

			list_add(reg->args, arg);

			contador_argumentos++;
		} // fin carga lista argumentos

		memcpy(&(reg->args->elements_count), buffer + desplazamiento, INT);
			desplazamiento += INT;

		memcpy(&reg->cantidad_vars, buffer + desplazamiento, INT);
			desplazamiento += INT;

		reg->vars = list_create();

		int contador_variables = 0;

		while(contador_variables < reg->cantidad_vars){

			variable *var = malloc(sizeof(variable));

			memcpy(&var->nombre, buffer + desplazamiento, CHAR);
				desplazamiento += CHAR;
			memcpy(&var->direccion, buffer + desplazamiento, DIRECCION);
				desplazamiento += DIRECCION;

			list_add(reg->vars, var);

			 contador_variables++;
		} // fin carga lista variables

		memcpy(&(reg->vars->elements_count), buffer + desplazamiento, INT);
			desplazamiento += INT;

		memcpy(&reg->retPos, buffer + desplazamiento, INT);
			desplazamiento += INT;
		memcpy(&reg->retVar, buffer + desplazamiento, DIRECCION);
			desplazamiento += DIRECCION;

		list_add(unPcb->indiceStack, reg); // agrego el registro cargado y avanzo al siguiente

		contador_registros_stack++;
	} // fin while

	// Copio el int 'elements_count' campo del índice de stack:
	memcpy(&(unPcb->indiceStack->elements_count), buffer + desplazamiento, INT);
		desplazamiento += INT;
	// fin carga índice stack

	// Copio el índice de etiquetas:
	unPcb->indiceEtiquetas = malloc(unPcb->tamanioIndiceEtiquetas);
	memcpy(unPcb->indiceEtiquetas, buffer + desplazamiento, unPcb->tamanioIndiceEtiquetas);

	return unPcb;
}

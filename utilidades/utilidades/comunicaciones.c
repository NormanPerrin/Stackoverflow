#include "comunicaciones.h"

// SEREALIZAR: Del mensaje listo para enviar, al buffer
// DESEREAILZAR: Del buffer, al mensaje listo para recibir

void aplicar_protocolo_enviar(int fdReceptor, int head, void *mensaje){

	int desplazamiento = 0, tamanioMensaje, tamanioTotalAEnviar;

	if (head < 1 || head > FIN_DEL_PROTOCOLO){
		//printf("Error al enviar paquete. No existe protocolo definido para %d\n", head);
		printf("Error al enviar paquete.");
		}
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

	// Validar contra NULL al recibir en cada módulo (lanzar un mensaje de error notificando)
	// Recibo primero el head:
	int recibido = recibirPorSocket(fdEmisor, head, INT);

	if (*head < 1 || *head > FIN_DEL_PROTOCOLO || recibido <= 0){
		//printf("Error al recibir paquete. No existe protocolo definido para %d\n", *head);
		printf("Error al recibir paquete.");
		return NULL;
	}

	// Recibo ahora el tamaño del mensaje:
	int* tamanioMensaje = malloc(INT);
	recibido = recibirPorSocket(fdEmisor, tamanioMensaje, INT);
		if (recibido <= 0){
			return NULL;
		}
	// Recibo por último el mensaje serealizado:
	void* mensaje = malloc(*tamanioMensaje);
	recibido = recibirPorSocket(fdEmisor, mensaje, *tamanioMensaje);
		if (recibido <= 0){
			return NULL;
		}
	// Deserealizo el mensaje según el protocolo:
	void* buffer = deserealizar(*head, mensaje, *tamanioMensaje);

	free(tamanioMensaje);
	free(mensaje);

	return buffer;
} // Se debe castear el mensaje al recibirse (indicar el tipo de dato que debe matchear con el void*)

int calcularTamanioMensaje(int head, void* mensaje){

	int tamanio;

	switch(head){

		// CASE 1: El mensaje es un texto (char*)
			case ENVIAR_SCRIPT: case IMPRIMIR_TEXTO: case DEVOLVER_INSTRUCCION: case WAIT_REQUEST:
			case SIGNAL_REQUEST: case OBTENER_VAR_COMPARTIDA: case DEVOLVER_PAGINA:{
				tamanio = strlen((char*)mensaje)+ 1;
				break;
			}
		// CASE 2: El mensaje es un texto (char*) más un valor entero (int)
			case ENTRADA_SALIDA: case GRABAR_VAR_COMPARTIDA:{
				pedidoIO* msj = (pedidoIO*)mensaje;
				tamanio = strlen(msj->nombreDispositivo)+ 5;
				break;
			}
		// CASE 3: El mensaje es un texto (char*) más dos valores enteros (int)
			case INICIAR_PROGRAMA: case ESCRIBIR_PAGINA:{
				inicioPrograma* msj = (inicioPrograma*)mensaje;
				tamanio = strlen(msj->contenido) + 2 * INT + 1;
				break;
			}
		// CASE 4: El mensaje es un pcb (pcb)
			case PCB: case PCB_FIN_EJECUCION: case PCB_FIN_QUANTUM: case PCB_ENTRADA_SALIDA:
			case PCB_WAIT:{
				tamanio = calcularTamanioPcb((pcb*) mensaje);
				break;
			}
		// CASE 5: El mensaje es un valor entero (int)
			case DEVOLVER_VARIABLE: case RESPUESTA_PEDIDO: case FINALIZAR_PROGRAMA: case IMPRIMIR:
			case PROGRAMA_NEW: case ABORTO_PROCESO: case INDICAR_PID: case DEVOLVER_VAR_COMPARTIDA:
			case TAMANIO_STACK: case SENIAL_SIGUSR1:{
				tamanio = 4;
				break;
				}
		// CASE 6: El mensaje son dos valores enteros (int)
			case LEER_PAGINA:{
				tamanio = 8;
				break;
			}
		// CASE 7: El mensaje son tres valores enteros (int)
			case PEDIDO_LECTURA_VARIABLE: case PEDIDO_ESCRITURA: case PEDIDO_LECTURA_INSTRUCCION:{
				tamanio = 12;
				break;
			}
		}
	return tamanio;
}

int calcularTamanioIndiceStack(pcb* unPcb){
	int i, tamanio = 0;
	/*if(list_is_empty(unPcb->indiceStack)){ // el índice está vacío
			return tamanio + 4; // Sumo 4 de elemnts_count (propia del índice)
			}
			else{ // hay ago en el índice*/
				for(i=0; i<list_size(unPcb->indiceStack); i++){
					registroStack* reg = list_get(unPcb->indiceStack, i);
					int tam_args = 4 + (list_size(reg->args)*14);
					int tam_vars = 4 + (list_size(reg->vars)*14);
					tamanio += (16 + tam_args + tam_vars); // Sumo además 12 bytes de retVar + 4 de retPos
				}
				return tamanio + 4;
			//}
}

int calcularTamanioPcb(pcb* unPcb){

	int tamanio;
	int stack_size = calcularTamanioIndiceStack(unPcb);
	// Sumo 60 bytes por los 15 int que tiene + los tamaños de los tres índices:
	tamanio = 60 + unPcb->tamanioIndiceEtiquetas + unPcb->tamanioIndiceCodigo + stack_size;

	return tamanio;
}

void * serealizar(int head, void * mensaje, int tamanio){

	void * buffer;

	switch(head){

	// CASE 1: El mensaje es un texto (char*)
	case ENVIAR_SCRIPT: case IMPRIMIR_TEXTO: case DEVOLVER_INSTRUCCION: case WAIT_REQUEST:
	case SIGNAL_REQUEST: case OBTENER_VAR_COMPARTIDA: case DEVOLVER_PAGINA:{
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
	case INICIAR_PROGRAMA: case ESCRIBIR_PAGINA:{
		buffer = serealizarTextoMasDosInt(mensaje, tamanio);
			break;
		}
	// CASE 4: El mensaje es un pcb (pcb)
	case PCB: case PCB_FIN_EJECUCION: case PCB_FIN_QUANTUM: case PCB_ENTRADA_SALIDA: case PCB_WAIT:{
		buffer = serealizarPcb(mensaje, tamanio);
			break;
		}
	// CASE 5: El mensaje es un valor entero (int)
	case DEVOLVER_VARIABLE: case RESPUESTA_PEDIDO: case FINALIZAR_PROGRAMA: case IMPRIMIR: case SENIAL_SIGUSR1:
	case PROGRAMA_NEW: case ABORTO_PROCESO: case INDICAR_PID: case DEVOLVER_VAR_COMPARTIDA:
	case WAIT_SIN_BLOQUEO: case WAIT_CON_BLOQUEO: case TAMANIO_STACK:{
		buffer = malloc(4);
		memcpy(buffer, mensaje, 4);
			break;
		}
	// CASE 6: El mensaje son dos valores enteros (int)
	case LEER_PAGINA:{
		buffer = serealizarDosInt(mensaje, 8);
			break;
		}
	// CASE 7: El mensaje son tres valores enteros (int)
	case PEDIDO_LECTURA_VARIABLE: case PEDIDO_LECTURA_INSTRUCCION: case PEDIDO_ESCRITURA:{
		buffer = serealizarTresInt(mensaje, 12);
			break;
		}
	}
	return buffer;
}

void * deserealizar(int head, void * buffer, int tamanio){

	void * mensaje;

	switch(head){

		// CASE 1: El mensaje es un texto (char*)
		case ENVIAR_SCRIPT: case IMPRIMIR_TEXTO: case DEVOLVER_INSTRUCCION: case WAIT_REQUEST:
		case SIGNAL_REQUEST: case OBTENER_VAR_COMPARTIDA: case DEVOLVER_PAGINA:{
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
		case INICIAR_PROGRAMA: {
			mensaje = deserealizarTextoMasDosInt(buffer, tamanio);
				break;
			}
		case ESCRIBIR_PAGINA:{
			mensaje = (solicitudEscribirPagina*)deserealizarTextoMasDosInt(buffer, tamanio);
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
		case PEDIDO_LECTURA_VARIABLE: case PEDIDO_LECTURA_INSTRUCCION: case PEDIDO_ESCRITURA:{
			mensaje = (solicitudLectura*)deserealizarTresInt(buffer, tamanio);
				break;
			}
		}
	return mensaje;
} // Se debe castear lo retornado (indicar el tipo de dato que debe matchear con el void*)

/**** SEREALIZACIONES PARTICULARES ****/
void* serealizarTextoMasUnInt(void* mensaje, int tamanio){

		pedidoIO* msj = (pedidoIO*) mensaje;
		int desplazamiento = 0;
		int string_size = strlen(msj->nombreDispositivo)+1;

		void * buffer = malloc(tamanio);
		memcpy(buffer + desplazamiento, &(msj->tiempo), INT);
			desplazamiento += INT;
		memcpy(buffer + desplazamiento, msj->nombreDispositivo, string_size);

		return buffer;
}

pedidoIO* deserealizarTextoMasUnInt(void* buffer, int tamanio){

	int desplazamiento = 0;
	int string_size = tamanio - INT;

		pedidoIO * msj = malloc(tamanio);
		memcpy(&msj->tiempo, buffer + desplazamiento, INT);
			desplazamiento += INT;
		msj->nombreDispositivo = (char*)reservarMemoria(string_size);
		memcpy(msj->nombreDispositivo, buffer + desplazamiento, string_size);

		return msj;
}

void* serealizarTextoMasDosInt(void* mensaje, int tamanio){

	inicioPrograma* msj = (inicioPrograma*) mensaje;
	int desplazamiento = 0;
	int string_size = strlen(msj->contenido) + 1;

		void * buffer = malloc(tamanio);
		memcpy(buffer + desplazamiento, &(msj->pid), INT);
			desplazamiento += INT;
		memcpy(buffer + desplazamiento, &(msj->paginas), INT);
			desplazamiento += INT;
		memcpy(buffer + desplazamiento, msj->contenido, string_size);

		return buffer;
}

inicioPrograma* deserealizarTextoMasDosInt(void* buffer, int tamanio){

	int desplazamiento = 0;
	int string_size = tamanio - 2*INT;

	inicioPrograma *msj = (inicioPrograma*)reservarMemoria(tamanio);
	memcpy(&msj->pid, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&msj->paginas, buffer + desplazamiento, INT);
		desplazamiento += INT;
	msj->contenido = (char*)reservarMemoria(string_size);
	memcpy(msj->contenido, buffer + desplazamiento, string_size);

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

	solicitudLeerPagina* msj = malloc(tamanio);
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

	direccion * msj = malloc(tamanio);
	memcpy(&msj->pagina, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&msj->offset, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&msj->size, buffer + desplazamiento, INT);

		return msj;
}

void * serealizarPcb(void * mensaje, int tamanio){

	pcb* unPcb = (pcb*) mensaje;
	int desplazamiento = 0;
	//int tam_elems_indiceStack = calcularTamanioIndiceStack(unPcb)-4; // Menos elements_count

	void * buffer = malloc(tamanio);
	memcpy(buffer + desplazamiento, &(unPcb->cantidad_instrucciones), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPcb->id_cpu), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPcb->indexActualStack), INT);
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
	memcpy(buffer + desplazamiento, &(unPcb->tamanioIndiceCodigo), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPcb->tamanioIndiceEtiquetas), INT);
		desplazamiento += INT;

	memcpy(buffer + desplazamiento, unPcb->indiceCodigo, unPcb->tamanioIndiceCodigo);
		desplazamiento += unPcb->tamanioIndiceCodigo;

	memcpy(buffer + desplazamiento, unPcb->indiceEtiquetas, unPcb->tamanioIndiceEtiquetas);
		desplazamiento += unPcb->tamanioIndiceEtiquetas;

// Itero la lista. Muevo registro por registro. Si la lista está vacía, no entra al while porque da NULL.
		void moverVariablesSer(variable* var){
			memcpy(buffer + desplazamiento, var->nombre, 2);
				desplazamiento +=  2;
			memcpy(buffer + desplazamiento, &var->direccion, 12);
				desplazamiento += 12;
		}
		int i;
		for(i=0; i<list_size(unPcb->indiceStack); i++){
				registroStack* reg = list_get(unPcb->indiceStack, i);
				list_iterate(reg->args, (void*) moverVariablesSer);
				list_iterate(reg->vars, (void*) moverVariablesSer);
				memcpy(buffer + desplazamiento, &reg->retPos, 4);
					desplazamiento += 4;
				memcpy(buffer + desplazamiento, &reg->retVar, 12);
					desplazamiento += 12;
			}
	// Por úlltimo, muevo este int campo de la lista:
	memcpy(buffer + desplazamiento, &(unPcb->indiceStack->elements_count), INT);

	return buffer;
}

pcb * deserealizarPcb(void * buffer, int tamanio){ // TODO: ver reservar memoria en punteros de estructura

	int desplazamiento = 0;
	pcb * unPcb = malloc(tamanio);
	int tam_elems_indiceStack = calcularTamanioIndiceStack((pcb*)buffer)-4;

	memcpy(&unPcb->cantidad_instrucciones, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&unPcb->id_cpu, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&unPcb->indexActualStack, buffer + desplazamiento, INT);
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
	memcpy(&unPcb->tamanioIndiceCodigo, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&unPcb->tamanioIndiceEtiquetas, buffer + desplazamiento, INT);
		desplazamiento += INT;

		unPcb->indiceCodigo = malloc(unPcb->tamanioIndiceCodigo);
	memcpy(unPcb->indiceCodigo, buffer + desplazamiento, unPcb->tamanioIndiceCodigo);
		desplazamiento += unPcb->tamanioIndiceCodigo;

		unPcb->indiceEtiquetas = malloc(unPcb->tamanioIndiceEtiquetas);
	memcpy(unPcb->indiceEtiquetas, buffer + desplazamiento, unPcb->tamanioIndiceEtiquetas);
		desplazamiento += unPcb->tamanioIndiceEtiquetas;

// Itero la lista. Muevo registro por registro. Si la lista está vacía, no entra al while porque da NULL.
		void moverVariablesDes(variable* var){
				var->nombre = malloc(2);
			memcpy(var->nombre, buffer + desplazamiento, 2);
				desplazamiento +=  2;
			memcpy(&var->direccion, buffer + desplazamiento, 12);
				desplazamiento += 12;
		}
		int i;
		for(i=0; i<list_size(unPcb->indiceStack); i++){
			registroStack* reg = list_get(unPcb->indiceStack, i);
			list_iterate(reg->args, (void*) moverVariablesDes);
			list_iterate(reg->vars, (void*) moverVariablesDes);
			memcpy(&reg->retPos, buffer + desplazamiento, 4);
				desplazamiento += 4;
			memcpy(&reg->retVar, buffer + desplazamiento, 12);
				desplazamiento += 12;
		}
	// Por úlltimo, muevo este int campo de la lista:
	memcpy(&(unPcb->indiceStack->elements_count), buffer + desplazamiento, INT);

		return unPcb;
}

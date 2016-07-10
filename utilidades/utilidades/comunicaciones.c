#include "comunicaciones.h"

// TODO: pedidoIO, string, var_compartida y devolverPaginaInstruccion son el mismo mensaje. Generalizar.

void aplicar_protocolo_enviar(int fdReceptor, int head, void *mensaje){

	int desplazamiento = 0, tamanioMensaje, tamanioTotalAEnviar;

	if (head < 1 || head > FIN_DEL_PROTOCOLO){
		printf("Error al enviar paquete. No existe protocolo definido para %d\n", head);
		abort(); // es abortivo
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
	int recibido;

	if (*head < 1 || *head > FIN_DEL_PROTOCOLO){
		printf("Error al recibir paquete. No existe protocolo definido para %d\n", *head);
		abort(); // es abortivo
	}
	// Recibo primero el head:
	recibido = recibirPorSocket(fdEmisor, head, INT);
		if (recibido <= 0){
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
			case SIGNAL_REQUEST: case OBTENER_VAR_COMPARTIDA:{
				tamanio = strlen((char*)mensaje)+ 1;
				break;
			}
		// CASE 2: El mensaje es un texto (char*) más un valor entero (int)
			case ENTRADA_SALIDA: case GRABAR_VAR_COMPARTIDA: case DEVOLVER_PAGINA:{
				pedidoIO* msj = (pedidoIO*)mensaje;
				tamanio = strlen(msj->nombreDispositivo)+ 5;
				break;
			}
		// CASE 3: El mensaje es un texto (char*) más dos valores enteros (int)
			case INICIAR_PROGRAMA: case ESCRIBIR_PAGINA:{
				inicioPrograma* msj = (inicioPrograma*)mensaje;
				tamanio = strlen(msj->contenido)+ 9;
				break;
			}
		// CASE 4: El mensaje es un pcb (pcb)
			case PCB: case PCB_FIN_EJECUCION: case PCB_FIN_QUANTUM: case PCB_ENTRADA_SALIDA:
			case PCB_WAIT:{
				tamanio = calcularTamanioPCB(mensaje);
				break;
			}
		// CASE 5: El mensaje es un valor entero (int)
			case DEVOLVER_VARIABLE: case RESPUESTA_PEDIDO: case FINALIZAR_PROGRAMA: case IMPRIMIR:
			case RECHAZAR_PROGRAMA: case ABORTO_PROCESO: case RESPUESTA_INICIO_PROGRAMA: case INDICAR_PID:
			case DEVOLVER_VAR_COMPARTIDA:{
				tamanio = 4;
				break;
				}
		// CASE 6: El mensaje son dos valores enteros (int)
			case LEER_PAGINA:{
				tamanio = 8;
				break;
			}
		// CASE 7: El mensaje son tres valores enteros (int)
			case PEDIDO_LECTURA_VARIABLE: case PEDIDO_LECTURA_INSTRUCCION:{
				tamanio = 12;
				break;
			}
		// CASE 8: El mensaje son cuatro valores enteros (int)
			case PEDIDO_ESCRITURA:{
				tamanio = 16;
				break;
			}
		}
	return tamanio;
}

int calcularTamanioPCB(void* mensaje){
	//pcb* pcb = (pcb*)mensaje;
	return TRUE;
}

void * serealizar(int head, void * mensaje, int tamanio){

	void * buffer;

	switch(head){

	// CASE 1: El mensaje es un texto (char*)
	case ENVIAR_SCRIPT: case IMPRIMIR_TEXTO: case DEVOLVER_INSTRUCCION: case WAIT_REQUEST:
	case SIGNAL_REQUEST: case OBTENER_VAR_COMPARTIDA:{
			buffer = malloc(tamanio);
			memcpy(buffer, mensaje, tamanio);
			break;
		}
	// CASE 2: El mensaje es un texto (char*) más un valor entero (int)
	case ENTRADA_SALIDA: case GRABAR_VAR_COMPARTIDA: case DEVOLVER_PAGINA:{
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
	case DEVOLVER_VARIABLE: case RESPUESTA_PEDIDO: case FINALIZAR_PROGRAMA: case IMPRIMIR:
	case RECHAZAR_PROGRAMA: case ABORTO_PROCESO: case RESPUESTA_INICIO_PROGRAMA: case INDICAR_PID:
	case DEVOLVER_VAR_COMPARTIDA:{
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
	case PEDIDO_LECTURA_VARIABLE: case PEDIDO_LECTURA_INSTRUCCION:{
		buffer = serealizarTresInt(mensaje, tamanio);
			break;
		}
	// CASE 8: El mensaje son cuatro valores enteros (int)
	case PEDIDO_ESCRITURA:{
		buffer = serealizarCuatroInt(mensaje, tamanio);
			break;
		}
	}
	return buffer;
}

void * deserealizar(int head, void * mensaje, int tamanio){

	void * buffer;

	switch(head){

		// CASE 1: El mensaje es un texto (char*)
		case ENVIAR_SCRIPT: case IMPRIMIR_TEXTO: case DEVOLVER_INSTRUCCION: case WAIT_REQUEST:
		case SIGNAL_REQUEST: case OBTENER_VAR_COMPARTIDA:{
			char* msj = malloc(tamanio);
			memcpy(msj, mensaje, tamanio);
			buffer = msj;
				break;
			}
		// CASE 2: El mensaje es un texto (char*) más un valor entero (int)
		case ENTRADA_SALIDA:{
				buffer = deserealizarTextoMasUnInt(mensaje, tamanio);
				break;
			}

		case GRABAR_VAR_COMPARTIDA:{
				buffer = (var_compartida*)deserealizarTextoMasUnInt(mensaje, tamanio);
				break;
		}
		case DEVOLVER_PAGINA:{
				buffer = (devolverPagina*)deserealizarTextoMasUnInt(mensaje, tamanio);
				break;
			}
		// CASE 3: El mensaje es un texto (char*) más dos valores enteros (int)
		case INICIAR_PROGRAMA: {
			buffer = deserealizarTextoMasDosInt(mensaje, tamanio);
				break;
			}
		case ESCRIBIR_PAGINA:{
			buffer = (solicitudEscribirPagina*)deserealizarTextoMasDosInt(mensaje, tamanio);
				break;
			}
		// CASE 4: El mensaje es un pcb (pcb)
		case PCB: case PCB_FIN_EJECUCION: case PCB_FIN_QUANTUM: case PCB_ENTRADA_SALIDA: case PCB_WAIT:{
			buffer = deserealizarPcb(mensaje, tamanio);
				break;
			}
		// CASE 5: El mensaje es un valor entero (int)
		case DEVOLVER_VARIABLE: case RESPUESTA_PEDIDO: case FINALIZAR_PROGRAMA: case IMPRIMIR:
		case RECHAZAR_PROGRAMA: case ABORTO_PROCESO: case RESPUESTA_INICIO_PROGRAMA: case INDICAR_PID:
		case DEVOLVER_VAR_COMPARTIDA:{
			int* msj = malloc(tamanio);
			memcpy(msj, mensaje, tamanio);
			buffer = msj;
				break;
			}
		// CASE 6: El mensaje son dos valores enteros (int)
		case LEER_PAGINA:{
			buffer = deserealizarDosInt(mensaje, tamanio);
				break;
			}
		// CASE 7: El mensaje son tres valores enteros (int)
		case PEDIDO_LECTURA_VARIABLE: case PEDIDO_LECTURA_INSTRUCCION:{
			buffer = (solicitudLectura*)deserealizarTresInt(mensaje, tamanio);
				break;
			}
		// CASE 8: El mensaje son cuatro valores enteros (int)
		case PEDIDO_ESCRITURA:{
			buffer = deserealizarCuatroInt(mensaje, tamanio);
				break;
			}
		}
	return buffer;
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

	pedidoIO * msj = malloc(tamanio);
	int desplazamiento = 0;
	int string_size = strlen(msj->nombreDispositivo)+1;

		memcpy(&msj->tiempo, buffer + desplazamiento, INT);
			desplazamiento += INT;
		memcpy(msj->nombreDispositivo, buffer + desplazamiento, string_size);

		return msj;
}

void* serealizarTextoMasDosInt(void* mensaje, int tamanio){

	inicioPrograma* msj = (inicioPrograma*) mensaje;
	int desplazamiento = 0;
	int string_size = strlen(msj->contenido)+1;

		void * buffer = malloc(tamanio);
		memcpy(buffer + desplazamiento, &(msj->paginas), INT);
			desplazamiento += INT;
		memcpy(buffer + desplazamiento, &(msj->pid), INT);
			desplazamiento += INT;
		memcpy(buffer + desplazamiento, msj->contenido, string_size);

		return buffer;
}

inicioPrograma* deserealizarTextoMasDosInt(void* buffer, int tamanio){

	int desplazamiento = 0;
	inicioPrograma * msj = malloc(tamanio);
	int string_size = strlen(msj->contenido)+1;

	memcpy(&msj->paginas, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&msj->pid, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(msj->contenido, buffer + desplazamiento, string_size);

		return msj;
}

void* serealizarDosInt(void* mensaje, int tamanio){

	solicitudLeerPagina* msj = (solicitudLeerPagina*) mensaje;
	int desplazamiento = 0;

	void * buffer = malloc(tamanio);
	memcpy(buffer + desplazamiento, &(msj->pagina), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(msj->pid), INT);

		return buffer;
}

solicitudLeerPagina* deserealizarDosInt(void* buffer, int tamanio){

	int desplazamiento = 0;
	solicitudLeerPagina * msj = malloc(tamanio);

	memcpy(&msj->pagina, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&msj->pid, buffer + desplazamiento, INT);

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

void* serealizarCuatroInt(void* mensaje, int tamanio){

	solicitudEscritura* msj = (solicitudEscritura*) mensaje;
	int desplazamiento = 0;

	void * buffer = malloc(tamanio);
	memcpy(buffer + desplazamiento, &(msj->pagina), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(msj->offset), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(msj->tamanio), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(msj->contenido), INT);

		return buffer;
}

solicitudEscritura* deserealizarCuatroInt(void* buffer, int tamanio){

	int desplazamiento = 0;
	solicitudEscritura * msj = malloc(tamanio);

	memcpy(&msj->pagina, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&msj->offset, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&msj->tamanio, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&msj->contenido, buffer + desplazamiento, INT);

		return msj;
}

void * serealizarPcb(void * estructura, int tamanio){
	/*pcb * unPCB = (pcb *) estructura;
	int tamanioTotal, desplazamiento = 0;

	int codigoSize = (sizeof(t_intructions) * unPCB->tamanioIndiceCodigo);

	int tamListaArgumentos = sizeof(direccion) * unPCB->indiceStack->tamanioArgumentos;
	int tamListaVariables = sizeof(t_dictionary) * unPCB->indiceStack->tamanioVariables;
	int tamRegistroStack = 6*INT + tamListaArgumentos + tamListaVariables;
	int stackSize = tamRegistroStack * unPCB->tamanioIndiceStack;

	int etiquetasSize = (CHAR * unPCB->tamanioIndiceEtiquetas);

	tamanioTotal = 9*INT + codigoSize + stackSize + etiquetasSize;

	void * buffer = reservarMemoria(tamanioTotal);
	memcpy(buffer + desplazamiento,&(unPCB->pid), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPCB->id_cpu), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPCB->pc), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPCB->paginas_codigo), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPCB->ultimaPosicionIndiceStack), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPCB->stackPointer), INT);
		desplazamiento += INT;

	// Serealizo el índice de código:
	memcpy(buffer + desplazamiento, &(unPCB->tamanioIndiceCodigo), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, unPCB->indiceCodigo, codigoSize);
		desplazamiento += codigoSize;

	// Serealizo el índice de etiquetas:
	memcpy(buffer + desplazamiento, &(unPCB->tamanioIndiceEtiquetas), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, unPCB->indiceEtiquetas, etiquetasSize);
		desplazamiento += etiquetasSize;

	// Serealizo el índice de stack:
	memcpy(buffer + desplazamiento, &(unPCB->tamanioIndiceStack), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPCB->indiceStack->posicionIndiceCodigo), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPCB->indiceStack->posicionResultado), sizeof(direccion));
		desplazamiento += sizeof(direccion);
	memcpy(buffer + desplazamiento, &(unPCB->indiceStack->tamanioArgumentos), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, unPCB->indiceStack->posicionesArgumentos, tamListaArgumentos);
		desplazamiento += tamListaArgumentos;
	memcpy(buffer + desplazamiento, &(unPCB->indiceStack->tamanioVariables), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, unPCB->indiceStack->variables, tamListaVariables);

	return buffer;*/
	void* r = NULL;
	return r;
}

pcb * deserealizarPcb(void * buffer, int tamanio){
	/*int desplazamiento = 0;
	pcb * unPcb = reservarMemoria(sizeof(pcb));

	memcpy(&unPcb->pid, buffer + desplazamiento, INT );
		desplazamiento += INT;
	memcpy(&unPcb->id_cpu, buffer + desplazamiento, INT );
		desplazamiento += INT;
	memcpy(&unPcb->pc, buffer + desplazamiento, INT );
		desplazamiento += INT;
	memcpy(&unPcb->paginas_codigo, buffer + desplazamiento, INT );
		desplazamiento += INT;
	memcpy(&unPcb->ultimaPosicionIndiceStack, buffer + desplazamiento, INT );
		desplazamiento += INT;
	memcpy(&unPcb->stackPointer, buffer + desplazamiento, INT );
		desplazamiento += INT;

	// Deserealizo el índice de código:
	memcpy(&unPcb->tamanioIndiceCodigo, buffer + desplazamiento, INT);
		desplazamiento += INT;
	unPcb->indiceCodigo = reservarMemoria(unPcb->tamanioIndiceCodigo);
	memcpy(unPcb->indiceCodigo, buffer + desplazamiento, unPcb->tamanioIndiceCodigo);
		desplazamiento += unPcb->tamanioIndiceCodigo;

		// Deserealizo el índice de etiquetas:
	memcpy(&unPcb->tamanioIndiceEtiquetas, buffer + desplazamiento, INT);
		desplazamiento += INT;
	unPcb->indiceEtiquetas = reservarMemoria(unPcb->tamanioIndiceEtiquetas);
	memcpy(unPcb->indiceEtiquetas, buffer + desplazamiento, unPcb->tamanioIndiceEtiquetas);
		desplazamiento += unPcb->tamanioIndiceEtiquetas;

	// Deserealizo el índice de stack:
	memcpy(&unPcb->tamanioIndiceStack, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&unPcb->indiceStack->posicionIndiceCodigo, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&unPcb->indiceStack->posicionResultado, buffer + desplazamiento, sizeof(direccion));
		desplazamiento += sizeof(direccion);
	memcpy(&unPcb->indiceStack->tamanioArgumentos, buffer + desplazamiento, INT);
			desplazamiento += INT;
	unPcb->indiceStack->posicionesArgumentos = reservarMemoria(unPcb->indiceStack->tamanioArgumentos);
	memcpy(unPcb->indiceStack->posicionesArgumentos, buffer + desplazamiento, unPcb->indiceStack->tamanioArgumentos);
		desplazamiento += unPcb->indiceStack->tamanioArgumentos;
	memcpy(&unPcb->indiceStack->tamanioVariables, buffer + desplazamiento, INT);
		desplazamiento += INT;
	unPcb->indiceStack->variables = reservarMemoria(unPcb->indiceStack->tamanioVariables);
	memcpy(unPcb->indiceStack->variables, buffer + desplazamiento, unPcb->indiceStack->tamanioVariables);

	return unPcb;*/
	pcb* r = NULL;
	return r;
}


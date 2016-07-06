#include "comunicaciones.h"

// TODO: pedidoIO, string, var_compartida y devolverPaginaInstruccion son el mismo mensaje. Generalizar.

void aplicar_protocolo_enviar(int fdReceptor, int protocolo, void *mensaje){

	int desplazamiento = 0, tamanioMensaje, tamanioTotal;

	if (protocolo < 1 || protocolo > FIN_DEL_PROTOCOLO){
		printf("Error al enviar paquete. No existe protocolo definido para %d\n", protocolo);
		abort(); // es abortivo
		}

	// Serealizar un mensaje dado un protocolo me devuelve el mensaje empaquetado:
	void *mensajeSerealizado = serealizar(protocolo, mensaje);
	tamanioMensaje = sizeof(mensajeSerealizado);
	/* Lo que se envía es: el protocolo + tamaño del msj + el msj serializado
	 * Entonces, el tamaño total de lo enviado es:
	 * 	16 bytes de los dos int + el tamaño del msj empaquetado */

	tamanioTotal = 2*INT + tamanioMensaje;

	// Meto en el paquete para enviar, esas tres cosas:
	void *buffer = malloc(tamanioTotal);
	memcpy(buffer + desplazamiento, &protocolo, INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &tamanioMensaje, INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, mensajeSerealizado, tamanioMensaje);

	// Se envía la totalidad del paquete (lo contenido en el buffer):
	enviarPorSocket(fdReceptor, buffer, tamanioTotal);

	free(buffer);
	free(mensajeSerealizado);
}

void * aplicar_protocolo_recibir(int fdEmisor, int * protocolo){

	// Recibo primero el head y lo verifico:
	int recibido = recibirPorSocket(fdEmisor, protocolo, INT);

	if (*protocolo < 1 || *protocolo > FIN_DEL_PROTOCOLO || recibido <= 0){
		return NULL; // Validar contra NULL al recibir en cada módulo (lanzar un mensaje de error notificando)
	}
	// Recibo ahora el tamaño del mensaje:
	int* tamanioMensaje = (int*)malloc(INT);
	recibirPorSocket(fdEmisor, tamanioMensaje, INT);

	// Recibo por último el mensaje serializado:
	void * mensaje = malloc(*tamanioMensaje);
	recibirPorSocket(fdEmisor, mensaje, *tamanioMensaje);

	// Deserealizo:
	void *buffer = deserealizar(*protocolo, mensaje);
	free(mensaje);
	free(tamanioMensaje);

	return buffer;

} // Se debe castear el mensaje al recibirse (indicar el tipo de dato que debe matchear con el void*)


void * serealizar(int protocolo, void * elemento){
	void * buffer;

	switch(protocolo){
		case ENVIAR_SCRIPT: case IMPRIMIR_TEXTO: case DEVOLVER_INSTRUCCION: case WAIT_REQUEST:
		case SIGNAL_REQUEST: case OBTENER_VAR_COMPARTIDA:{
			buffer = serealizarTexto(elemento);
			break;
		} // en todos los casos se envía un texto (char*)
		case ENTRADA_SALIDA: case GRABAR_VAR_COMPARTIDA:{
			//buffer = serealizarOperacionPrivilegiada(elemento);
			break;
		}
		case PCB: {
			buffer = serealizarPCB(elemento);
			break;
		}
		case INICIAR_PROGRAMA: {
			buffer = serealizarSolicitudInicioPrograma(elemento);
			break;
		}
		case DEVOLVER_VARIABLE: case RESPUESTA_PEDIDO: case FINALIZAR_PROGRAMA: case IMPRIMIR:
		case RECHAZAR_PROGRAMA: case PEDIDO_LECTURA_VARIABLE: case PEDIDO_ESCRITURA: case ABORTO_PROCESO:
		case ESCRIBIR_PAGINA: case RESPUESTA_INICIO_PROGRAMA: case QUANTUM_MODIFICADO:
		case LEER_PAGINA: case INDICAR_PID: case DEVOLVER_VAR_COMPARTIDA:
		case PEDIDO_LECTURA_INSTRUCCION:{
		// En estos casos se reciben elementos estáticos o estructuras con campos estáticos:
			int tamanio = sizeof(elemento);
			buffer = malloc(tamanio);
			memcpy(buffer, elemento, tamanio);
			break;
		}
		case DEVOLVER_PAGINA_INSTRUCCION:{
			buffer = serealizarDevolverPagina(elemento);
			break;
		}
		case 0:{
			printf("Se produjo una desconexión de sockets al serealizar\n"); // es el caso del recv = 0
			break;
		}
	}
	return buffer;
}

void * deserealizar(int protocolo, void * mensaje){
	void * buffer;

	switch(protocolo){
		case ENVIAR_SCRIPT: case IMPRIMIR_TEXTO: case DEVOLVER_INSTRUCCION: case WAIT_REQUEST:
		case SIGNAL_REQUEST: case OBTENER_VAR_COMPARTIDA:{
			buffer = deserealizarTexto(mensaje);
				break;
		} // en ambos casos se recibe un texto (char*)
		case PCB:{
			buffer = deserealizarPCB(mensaje);
			break;
		}
		case ENTRADA_SALIDA: case GRABAR_VAR_COMPARTIDA:{
			//buffer = deserealizarOperacionPrivilegiada(mensaje);
			break;
		}
		case INICIAR_PROGRAMA:{
			buffer = deserealizarSolicitudInicioPrograma(mensaje);
			break;
		}
		case DEVOLVER_VARIABLE: case RESPUESTA_PEDIDO: case FINALIZAR_PROGRAMA: case IMPRIMIR:
		case RECHAZAR_PROGRAMA: case PEDIDO_LECTURA_VARIABLE: case PEDIDO_ESCRITURA: case ABORTO_PROCESO:
		case ESCRIBIR_PAGINA: case RESPUESTA_INICIO_PROGRAMA: case QUANTUM_MODIFICADO:
		case LEER_PAGINA: case INDICAR_PID: case DEVOLVER_VAR_COMPARTIDA: case PEDIDO_LECTURA_INSTRUCCION:{
			// En estos casos se reciben elementos estáticos o estructuras con campos estáticos:
			int tamanio = sizeof(mensaje);
			buffer = reservarMemoria(tamanio);
			memcpy(buffer, mensaje, tamanio);
			break;
		}
		case DEVOLVER_PAGINA_INSTRUCCION:{
			buffer = deserializarDevolverPagina(mensaje);
			break;
		}
		case 0:{
			printf("Se produjo una desconexión de sockets al desearealizar\n");
			// es el caso del recv = 0
			break;
		}
	}
	return buffer;
} // Se debe castear lo retornado (indicar el tipo de dato que debe matchear con el void*)

/**** SERIALIZACIONES PARTICULARES ****/

// entra:	char *mensaje
// sale: 	int tamanio | char *mensaje
void *serealizarTexto(void *elemento) {

	char *mensaje = (char*)elemento;

	int desplazamiento = 0;
	int tamanioString = CHAR * strlen(mensaje);
	int tamanioTotal = INT + tamanioString;

	void *buffer = reservarMemoria(tamanioTotal);
	memcpy(buffer + desplazamiento, &tamanioString, INT);
	desplazamiento += INT;
	memcpy(buffer + desplazamiento, mensaje, tamanioString);

	return buffer;
}

// entra:	int tamanio | char *mensaje
// sale:	char *mensaje
void *deserealizarTexto(void *buffer) {

	int desplazamiento = 0, *tamanioString = NULL;

	memcpy(tamanioString, buffer + desplazamiento, INT);
	desplazamiento += INT;
	char *mensaje = (char*)reservarMemoria(*tamanioString);
	memcpy(mensaje, buffer + desplazamiento, *tamanioString);

	return mensaje;
}

void * serealizarPCB(void * estructura){
	pcb * unPCB = (pcb *) estructura;
	int tamanioTotal, desplazamiento = 0;

	int codigoSize = (sizeof(t_intructions) * unPCB->tamanioIndiceCodigo);

	int tamListaArgumentos = sizeof(direccion) * unPCB->indiceStack->tamanioArgumentos;
	int tamListaVariables = sizeof(t_dictionary) * unPCB->indiceStack->tamanioVariables;
	int tamRegistroStack = 6*INT + tamListaArgumentos + tamListaVariables;
	int stackSize = tamRegistroStack * unPCB->tamanioIndiceStack;

	int etiquetasSize = (CHAR * unPCB->tamanioIndiceEtiquetas);

	tamanioTotal = 10*INT + codigoSize + stackSize + etiquetasSize;

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
	memcpy(buffer + desplazamiento, &(unPCB->estado), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPCB->estado), INT);
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

	return buffer;
}

pcb * deserealizarPCB(void * buffer){
	int desplazamiento = 0;
	pcb * unPcb = reservarMemoria(sizeof(pcb));

	memcpy(&unPcb->pid, buffer + desplazamiento, INT );
		desplazamiento += INT;
	memcpy(&unPcb->id_cpu, buffer + desplazamiento, INT );
		desplazamiento += INT;
	memcpy(&unPcb->pc, buffer + desplazamiento, INT );
		desplazamiento += INT;
	memcpy(&unPcb->paginas_codigo, buffer + desplazamiento, INT );
		desplazamiento += INT;
	memcpy(&unPcb->estado, buffer + desplazamiento, INT );
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

	return unPcb;
}

// entra:	inicioPrograma *mensaje
// sale:	int pid | int paginas | int tamanio | char *contenido
void *serealizarSolicitudInicioPrograma(void *elemento) {

	inicioPrograma *mensaje = (inicioPrograma*)elemento;
	int tamanioTotal, desplazamiento = 0;

	int tamanioString = CHAR * strlen(mensaje->contenido);
	tamanioTotal = 3 * INT + tamanioString;

	void *buffer = reservarMemoria(tamanioTotal);

	memcpy(buffer + desplazamiento, &(mensaje->pid), INT);
	desplazamiento += INT;

	memcpy(buffer + desplazamiento, &(mensaje->paginas), INT);
	desplazamiento += INT;

	memcpy(buffer + desplazamiento, &tamanioString, INT);
	desplazamiento += INT;

	memcpy(buffer + desplazamiento, mensaje->contenido, tamanioString);

	return buffer;
}

// entra:	int pid | int paginas | int tamanio | char *contenido
// sale:	inicioPrograma *mensaje
void *deserealizarSolicitudInicioPrograma(void *buffer) {

	int desplazamiento = 0, *tamanioString = NULL;
	inicioPrograma *mensaje = (inicioPrograma*)reservarMemoria(sizeof(inicioPrograma));

	memcpy( &(mensaje->pid), buffer + desplazamiento, INT );
	desplazamiento += INT;
	memcpy( &(mensaje->paginas), buffer + desplazamiento, INT );
	desplazamiento += INT;
	memcpy( tamanioString, buffer + desplazamiento, INT );
	desplazamiento += INT;
	mensaje->contenido = (char*)reservarMemoria(*tamanioString);
	memcpy( mensaje->contenido, buffer + desplazamiento, *tamanioString);

	return mensaje;
}

// entra:	devolverPagina *elemento
// sale:	int pagina | int tamanio | char *contenido
void *serealizarDevolverPaginaInstruccion(void *elemento) {

	devolverPaginaInstruccion *mensaje = (devolverPaginaInstruccion*)elemento;
	int tamanioTotal, desplazamiento = 0;

	int tamanioString = CHAR * strlen(mensaje->contenido);
	tamanioTotal = 2 * INT + tamanioString;

	void *buffer = reservarMemoria(tamanioTotal);
	memcpy(buffer + desplazamiento, &(mensaje->pagina), INT);
	desplazamiento += INT;
	memcpy(buffer + desplazamiento, &tamanioString, INT);
	desplazamiento += INT;
	memcpy(buffer + desplazamiento, mensaje->contenido, tamanioString);

	return buffer;
}

// entra:	int pagina | int tamanio | char *contenido
// sale:	devolverPagina *elemento
void *deserializarDevolverPaginaInstruccion(void *buffer) {

	int desplazamiento = 0, *tamanioString = NULL;
	devolverPaginaInstruccion *mensaje = (devolverPaginaInstruccion*)reservarMemoria(sizeof(devolverPaginaInstruccion));

	memcpy( &(mensaje->pagina), buffer + desplazamiento, INT );
	desplazamiento += INT;
	memcpy( tamanioString, buffer + desplazamiento, INT );
	desplazamiento += INT;
	mensaje->contenido = (char*)reservarMemoria(*tamanioString);
	memcpy( mensaje->contenido, buffer + desplazamiento, *tamanioString);

	return mensaje;
}
/*
// Para grabar variable compartida y pedido de e/s:
void* serealizarOperacionPrivilegiada(void* elemento){

	// completar

}

// Para grabar variable compartida y pedido de e/s:
void* deserealizarOperacionPrivilegiada(void* buffer){

	// completar

}
*/

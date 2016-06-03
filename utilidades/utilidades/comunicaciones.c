#include "comunicaciones.h"

// TODO: Ver si usar uint16_t, sino sigamos siendo amigos del int.

void aplicar_protocolo_enviar(int fdCliente, int protocolo, void * mensaje){

	int desplazamiento = 0, tamanioTotal;
	int* tamanioMensaje = autoinicializado();
	int* head = autoinicializado();

	if (protocolo < 1 || protocolo > FIN_DEL_PROTOCOLO){
		fprintf(stderr, "Error al enviar paquete. No existe protocolo definido para %d\n", protocolo);
		abort(); // es abortivo
		}

	// Serealizar un mensaje dado un protocolo, me devuelve el mensaje empaquetado:
	void * mensajeSerealizado = serealizar(protocolo, mensaje);
	*tamanioMensaje = sizeof(mensajeSerealizado);
	*head = protocolo;

	/* Lo que se envía es: el protocolo + tamaño del msj + el msj serializado
	 * Entonces, el tamaño total de lo enviado es:
	 * 	16 bytes de los dos int + el tamaño del msj empaquetado */

	tamanioTotal = 2*INT + *tamanioMensaje;

	// Meto en el paquete para enviar, esas tres cosas:
	void * buffer = malloc(tamanioTotal);
	memcpy(buffer + desplazamiento, head, INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, tamanioMensaje, INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, mensajeSerealizado, *tamanioMensaje);

	// Se envía la totalidad del paquete (lo contenido en el buffer):
	enviarPorSocket(fdCliente, buffer, tamanioTotal);

	free(buffer);
	free(mensajeSerealizado);
	free(head);
	free(tamanioMensaje);
}

void * aplicar_protocolo_recibir(int fdCliente, int protocolo){
	int* head = (int*)malloc(INT);
	int* tamanioMensaje = (int*)malloc(INT);

	if (protocolo < 1 || protocolo > FIN_DEL_PROTOCOLO){
		fprintf(stderr, "Error al recibir paquete. No existe protocolo definido para %d\n", protocolo);
			abort(); // es abortivo
		}

	// Recibo primero el head y lo verifico:
	recibirPorSocket(fdCliente, head, INT);

	if(*head != protocolo){
		return NULL; // El protocolo indicado existe, pero no coincide con el recibido
		// Validar contra NULL al recibir en cada módulo (tirar un mensaje de error notificando)
	}

	// Recibo ahora el tamaño del mensaje:
	recibirPorSocket(fdCliente, tamanioMensaje, INT);
	// Recibo por último el mensaje serializado:
	void * mensaje = malloc(*tamanioMensaje);
	recibirPorSocket(fdCliente, mensaje, *tamanioMensaje);

	// Deserealizo:
	void * buffer = deserealizar(*head, mensaje);

	free(mensaje);
	free(head);
	free(tamanioMensaje);

	return buffer;

} // Se debe castear el mensaje al recibirse (indicar el tipo de dato que debe matchear con el void*)

void * serealizar(int protocolo, void * elemento){
	void * buffer;

	switch(protocolo){
		case ENVIAR_SCRIPT: case IMPRIMIR_TEXTO:{
			buffer = serealizarTexto(elemento);
			break;
		} // en ambos casos se envía un texto (char*)
		case PCB: {
			buffer = serealizarPCB(elemento);
			break;
		}
		case INICIAR_PROGRAMA: {
			buffer = serealizarSolicitudInicioPrograma(elemento);
			break;
		}
		case PEDIDO_ESCRITURA: {
			buffer = serealizarSolicitudEscritura(elemento);
			break;
		}
		case RESPUESTA_PEDIDO:{
			buffer = serializarRespuestaPedido(elemento);
			break;
		}
		case FIN_QUANTUM: case FINALIZAR_PROGRAMA: case IMPRIMIR: case RECHAZAR_PROGRAMA:
		case PEDIDO_LECTURA: case RESPUESTA_INICIO_PROGRAMA: {
			// En estos casos se envían elementos estáticos, calcularle tamaño antes y pasarlo
			int tamanio = sizeof(elemento);

			buffer = malloc(tamanio);
			memcpy(buffer, elemento, tamanio);
			break;
		}
		case LEER_PAGINA:{
			break;
		}
		case ESCRIBIR_PAGINA:{
			break;
				}
		case DEVOLVER_PAGINA:{
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
		case ENVIAR_SCRIPT: case IMPRIMIR_TEXTO:{
			buffer = deserealizarTexto(mensaje);
				break;
		} // en ambos casos se recibe un texto (char*)
		case PCB:{
			buffer = deserealizarPCB(mensaje);
			break;
		}
		case INICIAR_PROGRAMA:{
			buffer = deserealizarSolicitudInicioPrograma(mensaje);
			break;
		}
		case PEDIDO_ESCRITURA: {
			buffer = deserealizarSolicitudEscritura(mensaje);
			break;
		}
		case RESPUESTA_PEDIDO:{
			buffer = deserializarRespuestaPedido(mensaje);
			break;
		}
		case FIN_QUANTUM: case FINALIZAR_PROGRAMA: case IMPRIMIR: case RECHAZAR_PROGRAMA:
			case PEDIDO_LECTURA: case RESPUESTA_INICIO_PROGRAMA: {
			// En estos casos se reciben elementos estáticos, calcularle tamaño antes y pasarlo
			int tamanio = sizeof(mensaje);

			buffer = malloc(tamanio);
			memcpy(buffer, mensaje, tamanio);
			break;
		}
		case LEER_PAGINA:{
			break;
		}
		case ESCRIBIR_PAGINA:{
			break;
		}
		case DEVOLVER_PAGINA:{
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

int* autoinicializado(){
	int* tamanio = (int*)malloc(INT);
	*tamanio = 0;
	return tamanio;
}

void* serealizarTexto(void * elemento){
	string * texto = (string * ) elemento;
	int* tamanio = autoinicializado();

	int desplazamiento = 0;
	int tamanioString = (sizeof(char) * texto->tamanio);
	*tamanio = sizeof(string) - sizeof(int) + tamanioString;

		void * buffer = malloc(*tamanio);
		memcpy(buffer + desplazamiento, &(texto->tamanio), sizeof(int));
			desplazamiento += sizeof(int);
		memcpy(buffer + desplazamiento, texto->cadena, tamanioString);

	return buffer;
}

string * deserealizarTexto(void * buffer){
	int desplazamiento = 0;
	string * texto = malloc(sizeof(string));

		memcpy(&texto->tamanio, buffer + desplazamiento, sizeof(int) );
		desplazamiento += sizeof(int);
		texto->cadena = malloc(texto->tamanio);
		memcpy(texto->cadena, buffer + desplazamiento, texto->tamanio);

	return texto;
}

void * serealizarPCB(void * estructura){
	pcb * unPCB = (pcb *) estructura;
	int* tamanio = autoinicializado();

	int desplazamiento = 0;
	int iCodigoSize = (sizeof(t_intructions) * unPCB->indiceCodigo.tamanio);
	int iStackSize = (sizeof(registroStack) * unPCB->indiceStack.tamanio);
	int iEtiquetasSize = (sizeof(char) * unPCB->indiceEtiquetas.tamanio);
	*tamanio = sizeof(pcb) - sizeof(int) + iCodigoSize + iStackSize + iEtiquetasSize;

	void * buffer = malloc(*tamanio);
	memcpy(buffer + desplazamiento,&(unPCB->pid), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, &(unPCB->fdCPU), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, &(unPCB->pc), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, &(unPCB->paginas_codigo), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, &(unPCB->estado), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, &(unPCB->estado), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, &(unPCB->indiceCodigo.tamanio), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, unPCB->indiceCodigo.instrucciones, iCodigoSize);
	desplazamiento += iCodigoSize;
	memcpy(buffer + desplazamiento, &(unPCB->indiceEtiquetas.tamanio), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, unPCB->indiceEtiquetas.etiquetas, iEtiquetasSize);
	desplazamiento += iEtiquetasSize;
	memcpy(buffer + desplazamiento, &(unPCB->indiceStack.tamanio), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, unPCB->indiceStack.registros, iStackSize);

	return buffer;
}

pcb * deserealizarPCB(void * buffer){
	int desplazamiento = 0;
	pcb * unPcb = malloc(sizeof(pcb));

	memcpy(&unPcb->pid, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);
	memcpy(&unPcb->fdCPU, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);
	memcpy(&unPcb->pc, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);
	memcpy(&unPcb->paginas_codigo, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);
	memcpy(&unPcb->estado, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);

	memcpy(&unPcb->indiceEtiquetas.tamanio, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);
	unPcb->indiceEtiquetas.etiquetas= malloc(unPcb->indiceEtiquetas.tamanio * sizeof(char));
	memcpy(unPcb->indiceEtiquetas.etiquetas, buffer + desplazamiento, unPcb->indiceEtiquetas.tamanio);
	desplazamiento += unPcb->indiceEtiquetas.tamanio;

	memcpy(&unPcb->indiceCodigo.tamanio, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);
	unPcb->indiceCodigo.instrucciones= malloc(unPcb->indiceCodigo.tamanio * sizeof(t_intructions));
	memcpy(unPcb->indiceCodigo.instrucciones, buffer + desplazamiento, unPcb->indiceCodigo.tamanio);
	desplazamiento += unPcb->indiceCodigo.tamanio;

	memcpy(&unPcb->indiceStack.tamanio, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);
	unPcb->indiceStack.registros= malloc(unPcb->indiceStack.tamanio * sizeof(registroStack));
	memcpy(unPcb->indiceStack.registros, buffer + desplazamiento, unPcb->indiceStack.tamanio);

	return unPcb;
}

void * serealizarSolicitudEscritura(void * elemento){
	solicitudEscritura * unaSolicitud = (solicitudEscritura * ) elemento;
	int* tamanio = autoinicializado();

		int desplazamiento = 0;
		int tamanioString = (sizeof(char) * unaSolicitud->buffer.tamanio);
		*tamanio = sizeof(solicitudEscritura) - sizeof(int) + tamanioString;

		void * buffer = malloc(*tamanio);
		memcpy(buffer + desplazamiento,&(unaSolicitud->posicion), 3*INT);
		desplazamiento += 3*INT;
		memcpy(buffer + desplazamiento, &(unaSolicitud->buffer.tamanio), sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(buffer + desplazamiento, unaSolicitud->buffer.cadena, tamanioString);

	return buffer;
}

solicitudEscritura * deserealizarSolicitudEscritura(void * buffer){
	int desplazamiento = 0;
			solicitudEscritura * unaSolicitud = malloc(sizeof(solicitudEscritura));

		memcpy(&unaSolicitud->posicion, buffer + desplazamiento, 3*INT );
		desplazamiento += 3*INT;
		memcpy(&unaSolicitud->buffer.tamanio, buffer + desplazamiento, sizeof(int) );
		desplazamiento += sizeof(int);
		unaSolicitud->buffer.cadena = malloc(unaSolicitud->buffer.tamanio);
		memcpy(unaSolicitud->buffer.cadena, buffer + desplazamiento, unaSolicitud->buffer.tamanio);

	return unaSolicitud;
}

void * serializarRespuestaPedido(void * elemento){
	respuestaPedido* respuesta = (respuestaPedido*) elemento;
	int* tamanio = autoinicializado();

	int desplazamiento = 0;
	int tamanioExcepcion = (sizeof(char) * respuesta->mensaje.tamanio);
	int tamanioDataPedida = (sizeof(char) * respuesta->dataPedida.tamanio);
	*tamanio = sizeof(respuestaPedido) - 2*INT + tamanioExcepcion + tamanioDataPedida;

	void * buffer = malloc(*tamanio);
		memcpy(buffer + desplazamiento,&(respuesta->estadoPedido), sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(buffer + desplazamiento, respuesta->mensaje.cadena, tamanioExcepcion);
		desplazamiento += tamanioExcepcion;
		memcpy(buffer + desplazamiento, respuesta->dataPedida.cadena, tamanioDataPedida);

	return buffer;
}

respuestaPedido * deserializarRespuestaPedido(void * buffer){
	int desplazamiento = 0;
		respuestaPedido * respuesta = malloc(sizeof(respuestaPedido));

		memcpy(&respuesta->estadoPedido, buffer + desplazamiento, sizeof(int) );
		desplazamiento += sizeof(int);

		memcpy(&respuesta->mensaje.tamanio, buffer + desplazamiento, sizeof(int) );
		desplazamiento += sizeof(int);
		respuesta->mensaje.cadena = malloc(respuesta->mensaje.tamanio);
		memcpy(respuesta->mensaje.cadena, buffer + desplazamiento, respuesta->mensaje.tamanio);
		desplazamiento += respuesta->mensaje.tamanio;

		memcpy(&respuesta->dataPedida.tamanio, buffer + desplazamiento, sizeof(int) );
		desplazamiento += sizeof(int);
		respuesta->dataPedida.cadena = malloc(respuesta->dataPedida.tamanio);
		memcpy(respuesta->dataPedida.cadena, buffer + desplazamiento, respuesta->dataPedida.tamanio);

	return respuesta;
}

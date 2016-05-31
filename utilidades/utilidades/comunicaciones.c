#include "comunicaciones.h"

void aplicar_protocolo_enviar(int fdCliente, int head, void * mensaje, int tamanioMensaje){

	int desplazamiento = 0;

	void * mensajeSerealizado = serealizar(head, mensaje, &tamanioMensaje);

	int tamanioTotal = sizeof(int) * 2 + tamanioMensaje;

	void * buffer = malloc(tamanioTotal);
	memcpy(buffer + desplazamiento, &head, sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, &tamanioMensaje, sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, mensajeSerealizado, tamanioMensaje);

	// Se envía la totalidad del paquete (lo contenido en el buffer)
	enviarPorSocket(fdCliente, buffer, tamanioTotal);

	free(buffer);

	free(mensajeSerealizado);
}

void * aplicar_protocolo_recibir(int fdCliente, int * head, int * tamanioMensaje){
	// Recibo el head: devuelve NULL si el head no pertenece al protocolo, o bien, si falla el recv
	int recibido = recibirPorSocket(fdCliente, head, sizeof(int));

	if (*head < 1 || *head > FIN_DEL_PROTOCOLO || recibido <= 0){
		return NULL;
	}
	// Recibo el tamaño del mensaje
	recibirPorSocket(fdCliente, tamanioMensaje, sizeof(int));

	void * mensaje = malloc(*tamanioMensaje);
	recibirPorSocket(fdCliente, mensaje, *tamanioMensaje);
	void * buffer = deserealizar(*head, mensaje, *tamanioMensaje);
	free(mensaje);

	return buffer;

} // Se debe castear el mensaje al recibirse (indicar el tipo de dato que debe matchear con el void*)

void * serealizar(int protocolo, void * elemento, int * tamanio){
	void * buffer;

	switch(protocolo){
		case ENVIAR_SCRIPT: case IMPRIMIR_TEXTO:{
			buffer = serealizarTexto(elemento, tamanio);
			break;
		} // en ambos casos se envía un texto (char*)
		case ENVIAR_PCB: {
			buffer = serealizarPCB(elemento, tamanio);
			break;
		}
		case INICIAR_PROGRAMA: {
			buffer = serealizarSolicitudInicioPrograma(elemento, tamanio);
			break;
		}
		case RESPUESTA_INICIO_PROGRAMA: {
			buffer = serealizarRespuestaInicioPrograma(elemento,tamanio);
			break;
		}
		case LEER_BYTES: {
			buffer = serealizarDireccionMemoria(elemento,tamanio);
			break;
		}
		case ESCRIBIR_BYTES: case RESPUESTA_PEDIDO:{
			buffer = serealizarSolicitudEscritura(elemento, tamanio);
			break;
		} // se comportan de igual forma (mismo case)
		case FIN_QUANTUM: case FINALIZAR_PROGRAMA: case IMPRIMIR: case RECHAZAR_PROGRAMA:{
			buffer = malloc(*tamanio);
			memcpy(buffer, elemento, *tamanio);
			break; // en todos los casos se envía un valor entero (int)
		}
		case LEER_PAGINA:{
			break;
		}
		case ESCRIBIR_PAGINA:{
			break;
				}
		case DEVOLVER_BYTES:{
			break;
		}
		case DEVOLVER_PAGINA:{
			break;
		}
		case 0:{
			printf("Se produjo una desconexión de sockets\n"); // es el caso del recv = 0
			break;
		}
		default:{
			fprintf(stderr, "No existe protocolo definido para %d\n", protocolo);
			//abort();
			break;
		}
	}
	return buffer;
}

void * deserealizar(int protocolo, void * mensaje, int tamanio){
	void * buffer;

	switch(protocolo){
		case ENVIAR_SCRIPT: case IMPRIMIR_TEXTO:{
			buffer = deserealizarTexto(mensaje);
				break;
		} // en ambos casos se recibe un texto (char*)
		case ENVIAR_PCB:{
			buffer = deserealizarPCB(mensaje);
			break;
		}
		case INICIAR_PROGRAMA:{
			buffer = deserealizarSolicitudInicioPrograma(mensaje);
			break;
		}
		case RESPUESTA_INICIO_PROGRAMA: {
			buffer = deserealizarRespuestaInicioPrograma(mensaje);
			break;
		}
		case LEER_BYTES:{
			buffer = deserealizarDireccionMemoria(mensaje);
			break;
		}
		case ESCRIBIR_BYTES: case RESPUESTA_PEDIDO:{
			buffer = deserealizarSolicitudEscritura(mensaje);
			break;
		} // se comportan de igual forma (mismo case)
		case FIN_QUANTUM: case FINALIZAR_PROGRAMA: case IMPRIMIR: case RECHAZAR_PROGRAMA:{
			buffer = malloc(tamanio);
			memcpy(buffer, mensaje, tamanio);
			break;
		} // en todos los casos se recibe un valor entero (int)
		case LEER_PAGINA:{
			break;
		}
		case ESCRIBIR_PAGINA:{
			break;
		}
		case DEVOLVER_BYTES:{
			break;
				}
		case DEVOLVER_PAGINA:{
			break;
		}
		case 0:{
			printf("Se produjo una desconexión de sockets\n"); // es el caso del recv = 0
			break;
		}
		default:{
			fprintf(stderr, "No existe protocolo definido para %d\n", protocolo);
			//abort();
			break;
		}
	}
	return buffer;
} // Se debe castear lo retornado (indicar el tipo de dato que debe matchear con el void*)

void* serealizarTexto(void * estructura, int * tamanio){
t_string * unTexto = (t_string *) estructura;

int desplazamiento = 0;
	int tamanioTexto = (sizeof(char) * unTexto->tamanio);
	*tamanio = sizeof(t_string) - sizeof(int) + tamanioTexto;

	void * buffer = malloc(*tamanio);
	memcpy(buffer + desplazamiento, &(unTexto->tamanio), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, &(unTexto->texto), tamanioTexto);

	return buffer;
}

t_string * deserealizarTexto(void * buffer){
	int desplazamiento = 0;
	t_string * unTexto = malloc(sizeof(t_string));

	memcpy(&unTexto->tamanio, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);
	unTexto->texto= malloc(unTexto->tamanio * sizeof(char));
	memcpy(unTexto->texto, buffer + desplazamiento, unTexto->tamanio);

	return unTexto;
}

void * serealizarPCB(void * estructura, int * tamanio){
	pcb * unPCB = (pcb *) estructura;

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
	memcpy(buffer + desplazamiento, &(unPCB->quantum), sizeof(int));
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
	memcpy(&unPcb->quantum, buffer + desplazamiento, sizeof(int) );
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

void* serealizarSolicitudInicioPrograma(void* elemento, int* tamanio){
	iniciar_programa_t* solicitudInicio = (iniciar_programa_t*) elemento;

	int desplazamiento = 0;
	int tamanioCodigo = sizeof(char) * solicitudInicio->codigo.tamanio;
	*tamanio = sizeof(solicitudInicio) - sizeof(int) + tamanioCodigo;
	void* buffer = malloc(*tamanio);
	memcpy(buffer + desplazamiento,&solicitudInicio->pid,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, &solicitudInicio->paginas, sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, &(solicitudInicio->codigo.tamanio), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, solicitudInicio->codigo.texto, tamanioCodigo);

	return buffer;
}

iniciar_programa_t*  deserealizarSolicitudInicioPrograma(void* buffer){
	int desplazamiento = 0;
	iniciar_programa_t* solicitudInicio = malloc(sizeof(iniciar_programa_t));

	memcpy(&solicitudInicio->pid,buffer + desplazamiento,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(&solicitudInicio->paginas, buffer + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(&solicitudInicio->codigo.tamanio, buffer + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);
	solicitudInicio->codigo.texto= malloc(solicitudInicio->codigo.tamanio * sizeof(char));
	memcpy(&solicitudInicio->codigo.texto, buffer + desplazamiento, solicitudInicio->codigo.tamanio);

	return solicitudInicio;
}

void * serealizarRespuestaInicioPrograma(void * elemento, int * tamanio){
	respuestaInicioPrograma * respuesta = (respuestaInicioPrograma*) elemento;
	int desplazamiento = 0;
	*tamanio = sizeof(respuestaInicioPrograma) - sizeof(int);

	void* buffer = malloc(*tamanio);
	memcpy(buffer + desplazamiento,&respuesta->estadoDelHeap,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, &respuesta->pid, sizeof(int));

	return buffer;
}

respuestaInicioPrograma* deserealizarRespuestaInicioPrograma(void * buffer){
	int desplazamiento = 0;
	respuestaInicioPrograma* respuesta = malloc(sizeof(respuestaInicioPrograma));

	memcpy(&respuesta->estadoDelHeap,buffer + desplazamiento,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(&respuesta->pid, buffer + desplazamiento, sizeof(int));

	return respuesta;
}

void* serealizarDireccionMemoria(void* elemento, int* tamanio){
	direccion* solicitud = (direccion*) elemento;

	int desplazamiento = 0;
	*tamanio =sizeof(direccion);
	void* buffer = malloc(*tamanio);
	memcpy(buffer + desplazamiento,&solicitud->offset,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, &solicitud->pagina, sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, &solicitud->size, sizeof(int));

	return buffer;
}

direccion* deserealizarDireccionMemoria(void* buffer){
	int desplazamiento = 0;
	direccion* solicitud = malloc(sizeof(direccion));

	memcpy(&solicitud->offset,buffer + desplazamiento,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(&solicitud->pagina, buffer + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(&solicitud->size, buffer + desplazamiento, sizeof(int));

	return solicitud;
}

void * serealizarSolicitudEscritura(void * elemento, int * tamanio){
	solicitudEscritura * solicitud = (solicitudEscritura * ) elemento;

	int desplazamiento = 0;
	int tamanioDatos = (sizeof(char) * solicitud->buffer.tamanio);
	*tamanio = sizeof(solicitudEscritura) - sizeof(int) + tamanioDatos;

	void * buffer = malloc(*tamanio);
	memcpy(buffer + desplazamiento,&solicitud->posicion.offset,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, &solicitud->posicion.pagina, sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, &solicitud->posicion.size, sizeof(int));
	memcpy(buffer + desplazamiento, &(solicitud->buffer.tamanio), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, solicitud->buffer.texto, tamanioDatos);

	return buffer;
}

solicitudEscritura * deserealizarSolicitudEscritura(void * buffer){
	int desplazamiento = 0;
	solicitudEscritura * solicitud = malloc(sizeof(solicitudEscritura));

	memcpy(&solicitud->posicion.offset,buffer + desplazamiento,sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(&solicitud->posicion.pagina, buffer + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(&solicitud->posicion.size, buffer + desplazamiento, sizeof(int));
	memcpy(&solicitud->buffer.tamanio, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);
	solicitud->buffer.texto = malloc(solicitud->buffer.tamanio);
	memcpy(solicitud->buffer.texto, buffer + desplazamiento, solicitud->buffer.tamanio);

	return solicitud;
}

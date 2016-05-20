#include "comunicaciones.h"

void aplicar_protocolo_enviar(int fd, function protocolo, void *estructura) {

	void * msg_to_send;
	int length;

	switch(protocolo) {

		case IMPRIMIR:
			// trato imprimir

		case IMPRIMIR_TEXTO:
			// trato imprimir_texto

		case INICIAR_PROGRAMA:
			length = sizeof(iniciar_programa_t);
			msg_to_send = reservarMemoria(sizeof(iniciar_programa_t));
			enviarPorSocket(fd, msg_to_send, length);
			break;

		case LEER_BYTES:

		case ESCRIBIR_BYTES:

		case FINALIZAR_PROGRAMA:

		case ENVIAR_SCRIPT:

		case RESPUESTA_PEDIDO:

		case LEER_PAGINA:

		case ESCRIBIR_PAGINA:

		case DEVOLVER_BYTES:

		case DEVOLVER_PAGINA:

		case ENVIAR_PCB:
			length = sizeof(pcb);
			int * plength = (int *)length;
			msg_to_send = serealizarPCB(estructura, plength);
			enviarPorSocket(fd, msg_to_send, length);
			break;

		default:
			fprintf(stderr, "No existe protocolo definido para %d\n", protocolo);

	}

}



void *aplicar_protocolo_recibir(int fd, function protocolo) {

	int length;

	switch(protocolo) {

		case IMPRIMIR:
			length = msg_length(fd);
			char* msg = (char*)msg_content(fd, length+1);
			msg[length] = '\0';
//			imprimir(msg); // TODO revisar por qué falla
			free(msg);
			break;

		case IMPRIMIR_TEXTO:
			// trato imprimir_texto

		case INICIAR_PROGRAMA:
		{
			int pid, paginas, *ret;
			pid = msg_length(fd);
			paginas = msg_length(fd);
//			ret = inciar_programa(pid, paginas); // TODO ver por qué falla
			aplicar_protocolo_enviar(fd, RESPUESTA_PEDIDO, ret);
			break;
		}

		case LEER_BYTES:
		{
			int pid, paginas, offset;
			void *ret;
			pid = msg_length(fd);
			paginas = msg_length(fd);
			offset = msg_length(fd); // TODO ver de cuanto puede ser la página (size)
			// ret = leer_bytes(pid, pagina, offset);
			if(ret == NULL) {
				iniciar_programa_t *arg;
				arg->paginas = paginas;
				arg->pid = pid;
				aplicar_protocolo_enviar(fd, LEER_PAGINA, arg);
			}
			break;
		}

		case ESCRIBIR_BYTES:
		{
			int pid, paginas, offset;
			void *ret;
			pid = msg_length(fd);
			paginas = msg_length(fd);
			offset = msg_length(fd); // TODO ver de cuanto puede ser la página (size)
			// ret = escribir_bytes(pid, pagina, offset);
			aplicar_protocolo_enviar(fd, RESPUESTA_PEDIDO, ret);
			break;
		}

		case FINALIZAR_PROGRAMA:
		{
			int pid;
			pid = msg_length(fd);
			// finalizar_programa(pid);
			break;
		}

		case ENVIAR_SCRIPT:

		case RESPUESTA_PEDIDO:
		{
			int *respuesta = reservarMemoria(INT);
			*respuesta = msg_length(fd);
			return respuesta;
			break;
		}

		case LEER_PAGINA:

		case ESCRIBIR_PAGINA:
		{
			int pid, pagina, length, ret;
			void *contenido;
			pid = msg_length(fd);
			pagina = msg_length(fd);
			length = msg_length(fd);
			contenido = msg_content(fd, length);
			// ret = escribir_pagina(pid, pagina, contenido); // TODO
			aplicar_protocolo_enviar(fd, RESPUESTA_PEDIDO, ret);
			break;
		}

		case DEVOLVER_BYTES:

		case DEVOLVER_PAGINA:

		default:
			fprintf(stderr, "No existe protocolo definido para %d\n", protocolo);

	}

}


int msg_length(int fd) {

	uint8_t *buff = reservarMemoria(INT);
	recibirPorSocket(fd, buff, 1);

	int ret = *buff;
	free(buff);

	return ret;
}


void *msg_content(int fd, int length) {

	void *buff = reservarMemoria(length);
	recibirPorSocket(fd, buff, length);

	return buff;
}

// -- SERIALIZACIÓN/DESEAREALIZACIÓN DEL PCB --
void * serealizarPCB(void * estructura, int * tamanio){
	pcb * unPCB = (pcb *) estructura;

	int desplazamiento = 0;
	// *tamanio = sizeof(pcb) - y/o +

	void * buffer = malloc(*tamanio);
	memcpy(buffer + desplazamiento,&(unPCB->pid), sizeof(int));
	desplazamiento += sizeof(int); //pid
	memcpy(buffer + desplazamiento, &(unPCB->pc), sizeof(int));
	desplazamiento += sizeof(int);//pc
	memcpy(buffer + desplazamiento, &(unPCB->cantPaginas), sizeof(int));
	desplazamiento += sizeof(int);//cantidad de paginas
	// falta campo unPCB->indiceEtiquetas.elements
	memcpy(buffer + desplazamiento, &(unPCB->indiceEtiquetas.elements_amount), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, &(unPCB->indiceEtiquetas.table_current_size), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer + desplazamiento, &(unPCB->indiceEtiquetas.table_max_size), sizeof(int));
	desplazamiento += sizeof(int);
	// falta indice de stack
	// falta indice de codigo


	return buffer;
}

pcb * deserealizarPCB(void * buffer){
	int desplazamiento = 0;
	pcb * unPcb = malloc(sizeof(pcb));

	memcpy(&unPcb->pid, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int); //pid
	memcpy(&unPcb->pc, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int); //pc
	memcpy(&unPcb->cantPaginas, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);//cantidad paginas
		// falta indice de codigo
	desplazamiento += sizeof(int);
	memcpy(&unPcb->indiceEtiquetas.table_current_size, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);
	memcpy(&unPcb->indiceEtiquetas.table_max_size, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);
	// // falta campo unPCB->indiceEtiquetas.elements
	memcpy(&unPcb->indiceEtiquetas.elements_amount, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);
	memcpy(&unPcb->indiceEtiquetas.table_current_size, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int);
	memcpy(&unPcb->indiceEtiquetas.table_max_size, buffer + desplazamiento, sizeof(int) );
	desplazamiento += sizeof(int); //indice de etiqutas
	// falta indice de stack

	return unPcb;
}

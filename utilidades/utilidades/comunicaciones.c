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
		{
			length = sizeof(iniciar_programa_t);
			msg_to_send = reservarMemoria(sizeof(iniciar_programa_t));
			enviarPorSocket(fd, msg_to_send, length);
			break;
		}

		case LEER_BYTES:

		case ESCRIBIR_BYTES:

		case FINALIZAR_PROGRAMA:

		case ENVIAR_SCRIPT:
		{
			length = sizeof(t_string);
			int * plength = (int *)length;
			msg_to_send = serealizarScript(estructura, plength);
			enviarPorSocket(fd, msg_to_send, length);
			break;
		}

		case LEER_PAGINA:

		case ESCRIBIR_PAGINA:

		case DEVOLVER_BYTES:

		case DEVOLVER_PAGINA:

		case ENVIAR_PCB:
		{
			length = sizeof(pcb);
			int * plength = (int *)length;
			msg_to_send = serealizarPCB(estructura, plength);
			enviarPorSocket(fd, msg_to_send, length);
			break;
		}

		case FIN_QUANTUM:
		{
			// continuar
			break;
		}

		default:
		{
			fprintf(stderr, "No existe protocolo definido para %d\n", protocolo);
			break;
		}

	}

}



void* aplicar_protocolo_recibir(int fd, function protocolo) {

	int length;

	switch(protocolo) {

		case IMPRIMIR:
		{
			length = msg_length(fd);
			char* msg = (char*)msg_content(fd, length+1);
			msg[length] = '\0';
//			imprimir(msg); // TODO
			free(msg);
			break;
		}

		case IMPRIMIR_TEXTO:
			// trato imprimir_texto

		case INICIAR_PROGRAMA:
		{
			int pid, paginas, *ret;
			pid = msg_length(fd);
			paginas = msg_length(fd);
//			ret = inciar_programa(pid, paginas); // TODO
			aplicar_protocolo_enviar(fd, RESPUESTA_PEDIDO, ret);
			break;
		}

		case LEER_BYTES:
		{
			int pid, paginas, offset, tamanio;
			void *ret;
			pid = msg_length(fd);
			paginas = msg_length(fd);
			offset = msg_length(fd);
			tamanio = msg_length(fd);
			// ret = leer_bytes(pid, pagina, offset, tamanio); // TODO
			if(ret == NULL) {
				pedir_pagina(fd, pid, paginas);
			}
			break;
		}

		case ESCRIBIR_BYTES:
		{
			int pid, paginas, offset, tamanio;
			void *ret;
			pid = msg_length(fd);
			paginas = msg_length(fd);
			offset = msg_length(fd);
			tamanio = msg_length(fd);
			// ret = escribir_bytes(pid, pagina, offset, tamanio); // TODO
			aplicar_protocolo_enviar(fd, RESPUESTA_PEDIDO, ret); // TODO verificar ret antes
			break;
		}

		case FINALIZAR_PROGRAMA:
		{
			int pid;
			pid = msg_length(fd);
			// finalizar_programa(pid); // TODO: lo hace el Núcleo
			break;
		}

		case ENVIAR_SCRIPT:
		{
		// completar
			break;
		}

		case RESPUESTA_PEDIDO:
		{
			int *length = reservarMemoria(INT);
			void *respuesta;
			*length = msg_length(fd);
			respuesta = msg_content(fd ,length);
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
			aplicar_protocolo_enviar(fd, RESPUESTA_PEDIDO, ret); // TODO verificar ret antes
			break;
		}

		case DEVOLVER_BYTES:

		case DEVOLVER_PAGINA:

		case ENVIAR_PCB:
				{
					// completar
				break;
				}
		case FIN_QUANTUM:
				{
					// continuar
				break;
				}

		default:
		{
			fprintf(stderr, "No existe protocolo definido para %d\n", protocolo);
			break;
		}

	}

	return NULL;

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
	//tamanio = sizeof(pcb) - y/o +
	void * buffer = malloc(*tamanio);

	/*---- serializar pcb: memcpy del pcb al buffer ----*/

	return buffer;
}

pcb * deserealizarPCB(void * buffer){
	int desplazamiento = 0;
	pcb * unPcb = malloc(sizeof(pcb));

	/*---- deserializar pcb: memcpy del buffer al pcb ----*/

	return unPcb;
}

void * serealizarScript(void * codigo, int * size){
	t_string * unPCB = (t_string*) codigo;

		int desplazamiento = 0;
		//tamanio = sizeof(pcb) - y/o +
		void * buffer = malloc(*size);

		/*---- serializar script: memcpy del script al buffer ----*/

		return buffer;
}
t_string * deserealizarScript(void * buffer){
	int desplazamiento = 0;
		t_string * unScript = malloc(sizeof(t_string));

		/*---- deserializar script: memcpy del buffer al script ----*/

		return unScript;
}

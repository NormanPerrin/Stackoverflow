#include "comunicaciones.h"

void aplicar_protocolo_enviar(int fd, function protocolo, void *estructura) {

	void *msg_to_send;
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

		default:
			fprintf(stderr, "No existe protocolo definido para %d\n", protocolo);

	}

}



void aplicar_protocolo_recibir(int fd, function protocolo) {

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
			int pid, paginas;
			pid = msg_length(fd);
			paginas = msg_length(fd);
//			inciar_programa(pid, paginas); // TODO ver por qué falla
			break;
		}

		case LEER_BYTES:

		case ESCRIBIR_BYTES:

		case FINALIZAR_PROGRAMA:

		case ENVIAR_SCRIPT:

		case RESPUESTA_PEDIDO:

		case LEER_PAGINA:

		case ESCRIBIR_PAGINA:

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

#include "comunicaciones.h"

void aplicar_protocolo_enviar(int fd, function protocolo, void *estructura) {

	switch(protocolo) {

		case IMPRIMIR:
			// trato imprimir

			break;

		case IMPRIMIR_TEXTO:
			// trato imprimir_texto

		default:
			fprintf(stderr, "No existe protocolo definido para %d\n", protocolo);

	}

}



void aplicar_protocolo_recibir(int fd, function protocolo) {

	int length;

	switch(protocolo) {

		case IMPRIMIR:
			length = msg_length(fd);
			char *msg = (char*)msg_content(fd, length);
			printf("IMPRIMIR: %s\n", msg);
			free(msg);
			break;

		case IMPRIMIR_TEXTO:
			// trato imprimir_texto

		default:
			fprintf(stderr, "No existe protocolo definido para %d\n", protocolo);

	}

}


int msg_length(int fd) {

	int *buff = reservarMemoria(INT);
	recibirPorSocket(fd, buff, INT);

	int ret = *buff;
	free(buff);

	return ret;
}


void *msg_content(int fd, int length) {

	void *buff = reservarMemoria(length);
	recibirPorSocket(fd, buff, length);

	return buff;
}

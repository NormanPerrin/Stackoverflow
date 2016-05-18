#ifndef UTILIDADES_COMUNICACIONES_H_
#define UTILIDADES_COMUNICACIONES_H_

	#include "general.h"
	#include "sockets.h"

	// Estructuras
	typedef enum {
		IMPRIMIR = 1,
		IMPRIMIR_TEXTO = 2
		// hay que agregar las que faltan
	} function;

	// Cabeceras
	void aplicar_protocolo_enviar(int fd, function protocolo, void *estructura);
	void aplicar_protocolo_recibir(int fd, function protocolo);
	int msg_length(int fd); // devuelve el tamaño del mensaje
	void *msg_content(int fd, int length); // devuelve un puntero al contenido del mensaje

#endif /* UTILIDADES_COMUNICACIONES_H_ */

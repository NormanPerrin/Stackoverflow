#ifndef UTILIDADES_COMUNICACIONES_H_
#define UTILIDADES_COMUNICACIONES_H_

	#include "general.h"
	#include "sockets.h"

	// Estructuras
	typedef enum {
		IMPRIMIR = 1, 			// Núcleo - Consola
		IMPRIMIR_TEXTO = 2, 	// Núcleo - Consola
		INICIAR_PROGRAMA = 3, 	// Núcleo - UMC / UMC - Swap
		LEER_BYTES = 4, 		// Núcleo / CPU - UMC
		ESCRIBIR_BYTES = 5, 	// Núcleo / CPU - UMC
		FINALIZAR_PROGRAMA = 6, // Núcleo - UMC
		ENVIAR_SCRIPT = 7, 		// Consola - Núcleo
		RESPUESTA_PEDIDO = 10, 	// todos que reciban pedido
		LEER_PAGINA = 8, 		// UMC - Swap
		ESCRIBIR_PAGINA = 9, 	// UMC - Swap
		DEVOLVER_BYTES = 11, 	// UMC - Núcleo / CPU
		DEVOLVER_PAGINA = 12	// Swap - UMC
		// hay que agregar las que faltan
	} function;

	typedef struct {
		uint8_t pid;
		uint8_t paginas;
	} __attribute__((packed))
	iniciar_programa_t;

	// Cabeceras
	void aplicar_protocolo_enviar(int fd, function protocolo, void *estructura);
	void aplicar_protocolo_recibir(int fd, function protocolo);
	int msg_length(int fd); // devuelve el tamaño del mensaje
	void *msg_content(int fd, int length); // devuelve un puntero al contenido del mensaje
	// Funciones particulares de cada módulo // TODO las funciones particulares fallan porque no todos los tienen definidos... no se cual podría ser una solución
	void imprimir(char *msg);
	int inciar_programa(int pid, int paginas);

#endif /* UTILIDADES_COMUNICACIONES_H_ */

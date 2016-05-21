#ifndef UTILIDADES_COMUNICACIONES_H_
#define UTILIDADES_COMUNICACIONES_H_

	#include "general.h"
	#include "sockets.h"
	#include <commons/collections/list.h>
	#include <commons/collections/dictionary.h>

	#define TAM_MAX 50

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
		DEVOLVER_PAGINA = 12,	// Swap - UMC
		ENVIAR_PCB = 13,		// Núcleo - CPU / CPU - Núcleo
		FIN_QUANTUM = 14		// CPU - Núcleo
		// hay que agregar las que faltan
	} function;

	typedef struct {
			int pagina;
			int offset;
			int size;
	} t_direccion; // posición de memoria

	typedef struct {
		uint8_t pid;
		uint8_t paginas;
	} __attribute__((packed)) iniciar_programa_t;


// -- NÚCLEO - CPU --
	typedef struct {
		char id;
		t_direccion direccion;
	} t_variable;

	typedef struct {
		t_list direccionesArgumentos; // lista de t_direccion
		t_list variables; // lista de t_variable
		int proximoIndiceCodigo;
		t_direccion direccionVarRetorno;
	} registro_stack;

	typedef struct {
		int tamanio;
		t_list indice_stack; // lista de registro_stack
	} t_stack;

	typedef struct {
		int tamanio; // 8 bytes * cantidad de instrucciones
		int** indice_codigo; // matriz de 2(columnas: bytes comienzo, bytes fin) x cant. de instrucciones(filas)
	} t_codigo;

	typedef struct {
		int etiquetas_size;	// Tamaño del mapa serializado de etiquetas
		char* etiquetas; // La serializacion de las etiquetas
	} t_etiquetas;

	typedef struct pcb{
		int pid;
		int pc;
		int cantPaginas;
		t_codigo indiceCodigo;
		t_etiquetas indiceEtiquetas;
		t_stack indiceStack; // Indica qué variables hay en cada contexto y dónde están guardadas
		int baseStack; // ver si va acá
		int stackPointer;
		int estadoProceso; // NEW READY EXEC BLOCK EXIT
	} __attribute__((packed)) pcb;


	// Cabeceras
	void aplicar_protocolo_enviar(int fd, function protocolo, void *estructura);
	void *aplicar_protocolo_recibir(int fd, function protocolo);
	int msg_length(int fd); // devuelve el tamaño del mensaje
	void *msg_content(int fd, int length); // devuelve un puntero al contenido del mensaje

	// Funciones particulares de cada módulo // TODO las funciones particulares fallan porque no todos los tienen definidos... no se cual podría ser una solución
	void imprimir(char *msg);
	int inciar_programa(int pid, int paginas);
	void * serealizarPCB(void * estructura, int * size);
	pcb * deserealizarPCB(void * buffer);
	void *leer_bytes(int pid, int pagina, int offset, int tamanio);
	void pedir_pagina(int fd, int pid, int pagina);

#endif /* UTILIDADES_COMUNICACIONES_H_ */

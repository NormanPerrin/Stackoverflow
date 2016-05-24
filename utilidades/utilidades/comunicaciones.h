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
		FIN_QUANTUM = 14,		// CPU - Núcleo

		// hay que agregar las que faltan
	} function;

	typedef struct {
			int pagina;
			int offset;
			int size;
	} direccion; // posición de memoria

	typedef struct {
			int tamanio;
			char * texto;
		} t_string;

	typedef struct {
		int pid;
		int paginas;
		// + código del programa
	} __attribute__((packed)) iniciar_programa_t;

// -- Contenido del PCB --
	typedef struct {
		char id;
		direccion posicion;
	} variable;

	typedef struct {
		direccion* argumentos;
		variable* variablesLocales;
		int proximaInstruccion;
		direccion posicionDelResultado;
	} registroStack;

	typedef struct {
		int tamanio; // viene de config
		registroStack* registros;
	} stack;

	typedef struct {
		int comienzo;
		int longitud; // desplazamiento u offsset
	} instruccion;

	typedef struct {
		int tamanio; // 2 * sizeof(int) * cant de instrucciones
		instruccion* instrucciones;
	} codigo;

	typedef struct {
		int tamanio;	// Tamaño del mapa serializado de etiquetas
		char* etiquetas; // Serializacion de las etiquetas
	} etiquetas;

	typedef enum {
		NEW, READY, EXEC, BLOCK, EXIT
	} estadoProceso;

	typedef struct pcb{
		int pid;
		int pc;
		int paginas_codigo;
		codigo indiceCodigo;
		etiquetas indiceEtiquetas;
		stack indiceStack; // Indica qué variables hay en cada contexto y dónde están guardadas
		int* stackPointer; // SP del Stack
		int estado;
		int fdCPU;
		int quantum;
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
	void * serealizarScript(void * codigo, int * size);
	t_string * deserealizarScript(void * buffer);
	void *leer_bytes(int pid, int pagina, int offset, int tamanio);
	void pedir_pagina(int fd, int pid, int pagina);

#endif /* UTILIDADES_COMUNICACIONES_H_ */

#ifndef UTILIDADES_COMUNICACIONES_H_
#define UTILIDADES_COMUNICACIONES_H_

	#include "general.h"
	#include "sockets.h"
	#include <commons/collections/list.h>
	#include <commons/collections/dictionary.h>
	#include <parser/metadata_program.h>

/* *** IMPORTANTE - LEER ***
 *
 * PARA ENVIAR UN PAQUETE USAMOS:
 * void aplicar_protocolo_enviar(int fdReceptor, int protocolo, void* mensaje);
 * PRE-CONDICIONES: asegurarse de que el mensaje contenga los valores que queremos antes de enviarlo,
 * esti implica completar TODOS los campos, incluyendo aquellos que indiquen el tamaño para
 * elementos dinámicos.
 * POST-CONDICIONES: las acciones necesarias después del envío del msj se realizan en el respectivo módulo
 *
 * PARA RECIBIR UN PAQUETE USAMOS:
 * void * aplicar_protocolo_recibir(int fdEmisor, int protocolo);
 * PRE-CONDICIONES: tener alguna variable (creada dinámicamente con malloc, global, pasada por parámetro, etc.)
 * para poder asignarle lo recibido, casteándole el tipo de dato correspondiente (depende el caso, usar también memcpy)
 * POST-CONDICIONES: las acciones necesarias después de la recepción del msj se realizan en el respectivo módulo
 */

// PROTOCOLO (head/tipo de msj):
typedef enum {
		IMPRIMIR = 1, 					// Núcleo - Consola
		IMPRIMIR_TEXTO = 2, 			// Núcleo - Consola
		INICIAR_PROGRAMA = 3, 			// Núcleo - UMC / UMC - Swap
		PEDIDO_LECTURA = 4, 			// CPU - UMC
		PEDIDO_ESCRITURA = 5, 			// CPU - UMC
		FINALIZAR_PROGRAMA = 6,			// Núcleo - UMC
		ENVIAR_SCRIPT = 7, 				// Consola - Núcleo
		RESPUESTA_PEDIDO = 10, 			// UMC - CPU
		LEER_PAGINA = 8, 				// UMC - Swap
		ESCRIBIR_PAGINA = 9, 			// UMC - Swap
		DEVOLVER_PAGINA = 11,			// Swap - UMC
		PCB = 12,						// Núcleo - CPU / CPU - Núcleo
		FIN_QUANTUM = 13,				// CPU - Núcleo
		RESPUESTA_INICIO_PROGRAMA = 14, // UMC - Núcleo
		RECHAZAR_PROGRAMA = 15,			// Todos
		/*DEVOLVER_BYTES = 16, 			// UMC - Núcleo / CPU*/
		INDICAR_PID = 17, 				// CPU - UMC

		// hay que agregar las que falten...
		FIN_DEL_PROTOCOLO
	} protocolo;

// TADS para uso general
typedef struct {
		int pagina, offset, size;
} __attribute__((packed)) direccion; // posición de memoria, vale también para solicitud de Lectura

// TADS para texto variable y contenido PCB
typedef struct {
		int tamanio;
		char * cadena;
} __attribute__((packed)) string;

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
		int tamanio; // 2 * sizeof(int) * cant de instrucciones
		t_intructions* instrucciones;
	} codigo;

typedef struct {
	int tamanio;	// Tamaño del mapa serializado de etiquetas
	char* etiquetas; // Serializacion de las etiquetas
	} etiquetas;

// TADS para UMC - Núcleo
typedef struct {
		int pid, paginas;
		string codigo;
	} __attribute__((packed)) inicioPrograma;

typedef struct {
		int pid, estadoDelHeap;
// Si el heap es NO_CREADO, Núcleo rechaza acceso al sistema informando a Consola
	} __attribute__((packed)) respuestaInicioPrograma;

typedef enum {
	CREADO, NO_CREADO
}estadoDelHeap;

// TADS para Núcleo - CPU
typedef enum {
	NEW, READY, EXEC, BLOCK, EXIT
} estadoProceso;

typedef struct pcb{
		int pid, pc, paginas_codigo, estado, quantum, fdCPU;
		codigo indiceCodigo;
		etiquetas indiceEtiquetas;
		stack indiceStack; // Indica qué variables hay en cada contexto y dónde están guardadas
} __attribute__((packed)) pcb;

// TADS para UMC - CPU
typedef struct solicitudEscritura{
	direccion posicion;
	string buffer; // lo que se manda a escribir
} __attribute__((packed)) solicitudEscritura;

typedef struct {
	int estadoPedido;
	string mensaje; // msj de excepción ante solicitud
	string dataPedida; // instrucción si se lee código, variable si se lee stack, NULL si fue pedido escritura
}__attribute__((packed)) respuestaPedido;

typedef enum{
	PERMITIDO, NO_PERMITIDO // si es NO_PERMITIDO, UMC arroja escepción y se finaliza la ejecución
} estadoPedido;

// TADS para UMC - SWAP

/*** PROTOTIṔOS ***/
// -- Funciones definitivas para enviar y recibir PAQUETES:
void aplicar_protocolo_enviar(int fdReceptor, int protocolo, void * mensaje);
void* aplicar_protocolo_recibir(int fdEmisor, int protocolo);

// -- Serialización y deserialización GENERAL:
void * serealizar(int head, void * elemento);
void * deserealizar(int head, void * mensaje);

// -- Serializaciones y deserializaciones DINÁMICAS:
void* serealizarPCB(void* estructura);
pcb* deserealizarPCB(void* buffer);
void* serealizarTexto(void* estructura);
string* deserealizarTexto(void* buffer);
void* serealizarSolicitudInicioPrograma(void* elemento);
inicioPrograma* deserealizarSolicitudInicioPrograma(void* buffer);
void* serealizarSolicitudEscritura(void * elemento);
solicitudEscritura * deserealizarSolicitudEscritura(void * buffer);
void * serializarRespuestaPedido(void * elemento);
respuestaPedido * deserializarRespuestaPedido(void * buffer);

int* autoinicializado(); // retorna un puntero a una variable entera con valor 0

#endif /* UTILIDADES_COMUNICACIONES_H_ */

#ifndef UTILIDADES_COMUNICACIONES_H_
#define UTILIDADES_COMUNICACIONES_H_

	#include "general.h"
	#include "sockets.h"
	#include <commons/collections/list.h>
	#include <commons/collections/dictionary.h>
	#include <parser/metadata_program.h>

	#define SIZE_MSG 0

/* Para ENVIAR un mensaje, usamos:
 * void aplicar_protocolo_enviar(int fdCliente, int head, void* mensaje, int tamanioMensaje);
 * Pre-condiciones: asegurarse de que el mensaje contenga los valores que queremos antes de enviarlo
 * Post-condiciones: las acciones necesarias después del envío del msj se realizan en el respectivo módulo
 *
 * Para RECIBIR un mensaje, usamos:
 * void * aplicar_protocolo_recibir(int fdCliente, int* head, int* tamanioMensaje);
 * Pre-condiciones: tener una variable creada dinámicamente con malloc para poder asignarle lo recibido
 * Post-condiciones: las acciones necesarias después de la recepción del msj se realizan en el respectivo módulo
 *
 *OBS.: para los que requieren serialización, como argumento en tamaño del mensaje
 * 		usamos SIZE_MSG. Para los casos 1, 6 y 14 calculamos antes el tamaño y se lo pasamos
 */

// PROTOCOLO (heads):
typedef enum {
		IMPRIMIR = 1, 			// Núcleo - Consola
		IMPRIMIR_TEXTO = 2, 	// Núcleo - Consola
		INICIAR_PROGRAMA = 3, 	// Núcleo - UMC / UMC - Swap
		LEER_BYTES = 4, 		// Núcleo / CPU - UMC
		ESCRIBIR_BYTES = 5, 	// Núcleo / CPU - UMC
		FINALIZAR_PROGRAMA = 6, // Núcleo - UMC
		ENVIAR_SCRIPT = 7, 		// Consola - Núcleo
		RESPUESTA_PEDIDO = 10, 	// todos que reciban pedido de lectura o escritura
		LEER_PAGINA = 8, 		// UMC - Swap
		ESCRIBIR_PAGINA = 9, 	// UMC - Swap
		DEVOLVER_BYTES = 11, 	// UMC - Núcleo / CPU
		DEVOLVER_PAGINA = 12,	// Swap - UMC
		ENVIAR_PCB = 13,		// Núcleo - CPU / CPU - Núcleo
		FIN_QUANTUM = 14,		// CPU - Núcleo
		RESPUESTA_INICIO_PROGRAMA = 15, // UMC - Núcleo
		RECHAZAR_PROGRAMA = 16,
		// hay que agregar las que falten...

		FIN_DEL_PROTOCOLO
	} protocolo;

// Uso general
typedef struct {
		int pagina;
		int offset;
		int size;
	} __attribute__((packed)) direccion; // posición de memoria

// TADS (para texto variable y contenido PCB)
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

// UMC - Núcleo
typedef struct {
		int pid;
		int paginas;
		string codigo;
	} __attribute__((packed)) inicioPrograma;

typedef struct {
		int pid;
		int estadoDelHeap; // si es NO_CREADO, Núcleo rechaza acceso al sistema informando a Consola
	} __attribute__((packed)) respuestaInicioPrograma;

typedef enum {
		CREADO, NO_CREADO
	}estadoDelHeap;

// Núcleo - CPU
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
		int estado;
		int quantum; // TODO: lo borré, lo tengo que agregar a la serialización del PCB
		int fdCPU;
	} __attribute__((packed)) pcb;

// UMC - CPU
typedef struct solicitudEscritura{
	direccion posicion;
	string buffer;
} __attribute__((packed)) solicitudEscritura;

typedef struct {
	int estado;
	string mensaje;
	t_intructions instruccion;
}__attribute__((packed)) respuestaPedido;

typedef enum{
	PERMITIDO , NO_PERMITIDO
} estadoPedido;

// UMC - SWAP

// Prototipos
// -- Funciones definitivas para enviar y recibir paquetes:
void aplicar_protocolo_enviar(int fdCliente, int head, void * mensaje, int tamanioMensaje);
void* aplicar_protocolo_recibir(int fdCliente, int * head, int * tamanioMensaje);

// -- Serializaciones y deserializaciones particulares:
void* serealizarPCB(void* estructura, int* tamanio);
pcb* deserealizarPCB(void* buffer);

void* serealizarTexto(void* estructura, int* tamanio);
string* deserealizarTexto(void* buffer);

void* serealizarSolicitudInicioPrograma(void* elemento, int* tamanio);
inicioPrograma* deserealizarSolicitudInicioPrograma(void* buffer);

void* serealizarRespuestaInicioPrograma(void * elemento, int * tamanio);
respuestaInicioPrograma * deserealizarRespuestaInicioPrograma(void * buffer);

void* serealizarDireccionMemoria(void* elemento, int* tamanio);
direccion* deserealizarDireccionMemoria(void* buffer);

void* serealizarSolicitudEscritura(void * elemento, int * tamanio);
solicitudEscritura * deserealizarSolicitudEscritura(void * buffer);

void * serializarRespuestaPedido(void * elemento, int * tamanio);
respuestaPedido * deserializarRespuestaPedido(void * buffer);

// -- Serialización y deserialización que implementa las particulares:
void * serealizar(int head, void * elemento, int * tamanio);
void * deserealizar(int head, void * mensaje, int tamanio);

#endif /* UTILIDADES_COMUNICACIONES_H_ */

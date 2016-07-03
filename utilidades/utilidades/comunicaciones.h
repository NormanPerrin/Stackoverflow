#ifndef UTILIDADES_COMUNICACIONES_H_
#define UTILIDADES_COMUNICACIONES_H_

	#include "general.h"
	#include "sockets.h"
	#include <commons/collections/list.h>
	#include <commons/collections/dictionary.h>
	#include <parser/metadata_program.h>
	#define STRING (sizeof(string))

/* *** IMPORTANTE - LEER ***
 *
 * PARA ENVIAR UN PAQUETE USAMOS:
 * void aplicar_protocolo_enviar(int fdReceptor, int protocolo, void* mensaje);
 * PRE-CONDICIONES: asegurarse de que el mensaje contenga los valores que queremos antes de enviarlo,
 * esto implica completar TODOS los campos (incluyendo los NULL y aquellos que indiquen el tamaño para
 * elementos dinámicos).
 * POST-CONDICIONES: las acciones necesarias después del envío del msj se realizan en el respectivo módulo
 *
 * PARA RECIBIR UN PAQUETE USAMOS:
 * void * aplicar_protocolo_recibir(int fdEmisor, int * protocolo);
 * PRE-CONDICIONES: tener alguna variable (creada dinámicamente con malloc, global, pasada por parámetro, etc.)
 * para poder asignarle lo recibido, casteándole el tipo de dato correspondiente (depende el caso, usar también memcpy)
 * POST-CONDICIONES: las acciones necesarias después de la recepción del msj se realizan en el respectivo módulo
 */

// PROTOCOLO (head/tipo de msj):
typedef enum {
	// Mensajes Estáticos:
		IMPRIMIR = 1, 				// CPU - Núcleo
		RECHAZAR_PROGRAMA,			// Todos
		PEDIDO_LECTURA, 			// CPU - UMC
		PEDIDO_ESCRITURA, 			// CPU - UMC
		FINALIZAR_PROGRAMA,			// Núcleo - UMC / UMC - SWAP
		RESPUESTA_INICIO_PROGRAMA,	// UMC - Núcleo / Swap - UMC
		INDICAR_PID, 				// CPU - UMC
		QUANTUM_MODIFICADO,			// Núcleo - CPU
		LEER_PAGINA, 				// UMC - Swap
		ESCRIBIR_PAGINA, 			// UMC - Swap
		DEVOLVER_VARIABLE, 		// UMC - CPU
		RESPUESTA_PEDIDO, 			// UMC - CPU
		DEVOLVER_PAGINA_VARIABLE,	// Swap - UMC

		// Mensajes Dinámicos;
		INICIAR_PROGRAMA, 			// Núcleo - UMC / UMC - Swap
		ENVIAR_SCRIPT, 				// Consola - Núcleo
		PCB,						// Núcleo - CPU / CPU - Núcleo
		IMPRIMIR_TEXTO, 			// CPU - Núcleo / Núcleo - Consola
		DEVOLVER_INSTRUCCION,			// UMC - CPU
		DEVOLVER_PAGINA_INSTRUCCION,	// Swap - UMC

		// hay que agregar las que falten...
		FIN_DEL_PROTOCOLO
} protocolo;

// TADS para uso general:
typedef struct {
		int pagina, offset, size;
} __attribute__((packed)) direccion; // direccion lógica

// TADS para mensajes entre módulos:
typedef struct {
		int tamanio;
		char * cadena;
} __attribute__((packed)) string;

typedef struct{
	int quantum, retardoQuantum;
}__attribute__((packed)) info_quantum;

/*typedef struct {
		char id;
		direccion posicion;
} variable;*/

typedef struct {
		int tamanioListaArgumentos, tamanioListaVariables, proximaInstruccion;
		direccion* listaPosicionesArgumentos; // lista del tipo de dato 'direccion'
		t_dictionary* listaVariablesLocales; // diccionario: key: id -> data: dirección
		direccion posicionDelResultado; // página, offset, size
	} registroStack;

typedef struct {
	int pid, paginas;
	char *contenido;
} __attribute__((packed)) inicioPrograma;

typedef struct {
		int pid, estadoDelHeap;
// Si el heap es NO_CREADO, Núcleo rechaza acceso al sistema informando a Consola
	} __attribute__((packed)) respuestaInicioPrograma;

typedef enum {
	CREADO, NO_CREADO
} estadoDelHeap;

// TADS para Núcleo - CPU
typedef enum {
	NEW, READY, EXEC, BLOCK, EXIT
} estadoProceso;

typedef struct pcb{
	int pid, pc, paginas_codigo, estado, id_cpu;
	int tamanioIndiceCodigo, tamanioIndiceStack, tamanioIndiceEtiquetas;
	t_intructions* indiceCodigo;
	char* indiceEtiquetas; // Serializacion de las etiquetas
	registroStack* indiceStack; // Indica qué variables hay en cada contexto y dónde están guardadas
} __attribute__((packed)) pcb;

typedef struct {
	int pagina, offset, tamanio, contenido; // dirección lógica + variable
} __attribute__((packed)) solicitudEscritura;

typedef struct {
	int pagina, offset, tamanio; // dirección lógica
} __attribute__((packed)) solicitudLectura;

typedef struct {
	int pid, pagina;
} __attribute__((packed))solicitudLeerPagina;

typedef struct {
	int pid, pagina, contenido;
} __attribute__((packed))solicitudEscribirPagina;

typedef struct {
	int pagina;
	char *contenido;
} __attribute__((packed))devolverPaginaInstruccion;

typedef struct {
	int pagina, contenido;
} __attribute__((packed))devolverPaginaVariable;

typedef enum{
	PERMITIDO, NO_PERMITIDO // si es NO_PERMITIDO, UMC arroja escepción y se finaliza la ejecución
} estadoPedido;

/*** PROTOTIPOS ***/
// -- Funciones definitivas para enviar y recibir PAQUETES:
void aplicar_protocolo_enviar(int fdReceptor, int protocolo, void * mensaje);
void* aplicar_protocolo_recibir(int fdEmisor, int * protocolo);

// -- Serialización y deserialización GENERAL:
void * serealizar(int head, void * elemento);
void * deserealizar(int head, void * mensaje);

// -- Serializaciones y deserializaciones DINÁMICAS:
void* serealizarPCB(void* estructura);
pcb* deserealizarPCB(void* buffer);
void* serealizarTexto(void* estructura);
void* deserealizarTexto(void* buffer);
void* serealizarSolicitudInicioPrograma(void* elemento);
void* deserealizarSolicitudInicioPrograma(void* buffer);
void* serealizarSolicitudEscritura(void * elemento);
void * deserealizarSolicitudEscritura(void * buffer);
void *deserializarDevolverPagina(void *buffer);
void *serealizarDevolverPagina(void *elemento);
void *deserializarEscribirPagina(void * buffer);
void *serealizarEscribirPagina(void *elemento);


#endif /* UTILIDADES_COMUNICACIONES_H_ */

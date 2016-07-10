#ifndef UTILIDADES_COMUNICACIONES_H_
#define UTILIDADES_COMUNICACIONES_H_

	#include "general.h"

// TODO: pedidoIO, string, var_compartida y devolverPaginaInstruccion son el mismo mensaje. Generalizar.

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

/*** PROTOCOLO ***/
typedef enum {
	// Mensajes Estáticos:
	IMPRIMIR = 1, 					// CPU - Núcleo / Núcleo - Consola
	RECHAZAR_PROGRAMA,				// Núcleo - Consola
	PEDIDO_LECTURA_INSTRUCCION, 	// CPU - UMC
	PEDIDO_LECTURA_VARIABLE, 		// CPU - UMC
	INDICAR_PID, 					// CPU - UMC
	PEDIDO_ESCRITURA,				// CPU - UMC
	FINALIZAR_PROGRAMA,				// Núcleo - UMC / UMC - SWAP
	RESPUESTA_INICIO_PROGRAMA,		// UMC - Núcleo / Swap - UMC
	LEER_PAGINA, 					// UMC - Swap
	ESCRIBIR_PAGINA,				// UMC - Swap
	DEVOLVER_VARIABLE, 				// UMC - CPU
	RESPUESTA_PEDIDO, 				// UMC - CPU
	DEVOLVER_PAGINA_VARIABLE,		// Swap - UMC
	ABORTO_PROCESO,					// CPU - Núcleo
	WAIT_SIN_BLOQUEO, 				// Núcleo - CPU
	WAIT_CON_BLOQUEO,				// Núcleo - CPU
	DEVOLVER_VAR_COMPARTIDA,		// Núcleo - CPU

	SIGUSR,							// CPU - Núcleo

	// Mensajes Dinámicos;
	INICIAR_PROGRAMA, 				// Núcleo - UMC / UMC - Swap
	ENVIAR_SCRIPT, 					// Consola - Núcleo
	PCB, 							// Núcleo - CPU
	PCB_FIN_EJECUCION,				// CPU - Núcleo
	PCB_FIN_QUANTUM,				// CPU - Núcleo
	PCB_ENTRADA_SALIDA,				// CPU - Núcleo
	PCB_WAIT,						// CPU - Núcleo
	OBTENER_VAR_COMPARTIDA,			// CPU - Núcleo
	IMPRIMIR_TEXTO, 				// CPU - Núcleo / Núcleo - Consola
	DEVOLVER_INSTRUCCION,			// UMC - CPU
	DEVOLVER_PAGINA_INSTRUCCION,	// Swap - UMC
	WAIT_REQUEST, SIGNAL_REQUEST,	// CPU - Núcleo
	ENTRADA_SALIDA,					// CPU - Núcleo
	GRABAR_VAR_COMPARTIDA,			// CPU - Núcleo

	FIN_DEL_PROTOCOLO
} protocolo;

/*** PROTOTIPOS ***/
// -- Funciones definitivas para enviar y recibir PAQUETES:
void aplicar_protocolo_enviar(int fdReceptor, int protocolo, void * mensaje);
void* aplicar_protocolo_recibir(int fdEmisor, int * protocolo);

// -- Serialización y deserialización GENERAL:
void * serealizar(int head, void * elemento);
void * deserealizar(int head, void * mensaje);

// -- Serializaciones y deserializaciones DINÁMICAS:
void* serealizarPCB(void* elemento);
pcb* deserealizarPCB(void* buffer);
void* serealizarTexto(void* elemento);
void* deserealizarTexto(void* buffer);
void* serealizarSolicitudInicioPrograma(void* elemento);
void* deserealizarSolicitudInicioPrograma(void* buffer);
void* serealizarSolicitudEscritura(void * elemento);
void * deserealizarSolicitudEscritura(void * buffer);
void *deserializarDevolverPagina(void *buffer);
void *serealizarDevolverPagina(void *elemento);
void *deserializarEscribirPagina(void * buffer);
void *serealizarEscribirPagina(void *elemento);
void* serealizarOperacionPrivilegiada(void* elemento);
void* deserealizarOperacionPrivilegiada(void* buffer);

#endif /* UTILIDADES_COMUNICACIONES_H_ */

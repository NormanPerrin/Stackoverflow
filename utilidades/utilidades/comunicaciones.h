#ifndef UTILIDADES_COMUNICACIONES_H_
#define UTILIDADES_COMUNICACIONES_H_

#include "general.h"

#define NUM_ELEM(x) (sizeof (x) / sizeof (*(x)))
#define MSJ_VACIO ((void *)1) // Para no poner NULL

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
	DEVOLVER_VARIABLE, 				// UMC - CPU
	RESPUESTA_PEDIDO, 				// UMC - CPU
	ABORTO_PROCESO,					// CPU - Núcleo
	WAIT_SIN_BLOQUEO, 				// Núcleo - CPU
	WAIT_CON_BLOQUEO,				// Núcleo - CPU
	DEVOLVER_VAR_COMPARTIDA,		// Núcleo - CPU
	TAMANIO_STACK,					// Núcleo - CPU

	SIGUSR,							// CPU - Núcleo // agregar al switch

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
	DEVOLVER_PAGINA,				// Swap - UMC
	WAIT_REQUEST, SIGNAL_REQUEST,	// CPU - Núcleo
	ENTRADA_SALIDA,					// CPU - Núcleo
	GRABAR_VAR_COMPARTIDA,			// CPU - Núcleo
	ESCRIBIR_PAGINA,				// UMC - Swap

	FIN_DEL_PROTOCOLO
} protocolo;

/*** PROTOTIPOS ***/
// -- Funciones definitivas para enviar y recibir PAQUETES:
void aplicar_protocolo_enviar(int fdReceptor, int head, void * mensaje);
void* aplicar_protocolo_recibir(int fdEmisor, int* head);

int calcularTamanioMensaje(int head, void* mensaje);
int calcularTamanioPCB(void* mensaje);

// -- Serealización y deserealización GENERAL:
void * serealizar(int head, void * mensaje, int tamanio);
void * deserealizar(int head, void * buffer, int tamanio);

// -- Serealizaciones y deserealizaciones DINÁMICAS:
void* serealizarPcb(void* mensaje, int tamanio);
pcb* deserealizarPcb(void* buffer, int tamanio);
void* serealizarTextoMasUnInt(void* mensaje, int tamanio);
pedidoIO* deserealizarTextoMasUnInt(void* buffer, int tamanio);
void* serealizarTextoMasDosInt(void* buffer, int tamanio);
inicioPrograma* deserealizarTextoMasDosInt(void* buffer, int tamanio);
void* serealizarDosInt(void* mensaje, int tamanio);
solicitudLeerPagina* deserealizarDosInt(void* buffer, int tamanio);
void* serealizarTresInt(void* mensaje, int tamanio);
direccion* deserealizarTresInt(void* buffer, int tamanio);
void* serealizarCuatroInt(void* mensaje, int tamanio);
solicitudEscritura* deserealizarCuatroInt(void* buffer, int tamanio);
int calcularTamanioIndiceStack(t_list* indice);
void calcularTamanioRegistroStack(void* element);

#endif /* UTILIDADES_COMUNICACIONES_H_ */

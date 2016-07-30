#ifndef UTILIDADES_COMUNICACIONES_H_
#define UTILIDADES_COMUNICACIONES_H_

#include "general.h"

#define NUM_ELEM(x) (sizeof (x) / sizeof (*(x)))

// ENVIAR UN PAQUETE: void aplicar_protocolo_enviar(int fdReceptor, int protocolo, void* mensaje);
// RECIBIR UN PAQUETE: void * aplicar_protocolo_recibir(int fdEmisor, int* protocolo);

/*** PROTOCOLO ***/
typedef enum {
	// Mensajes de un solo elemento:
	IMPRIMIR = 1, 					// CPU - Núcleo / Núcleo - Consola
	PROGRAMA_NEW,					// Núcleo - Consola
	PEDIDO_LECTURA_INSTRUCCION, 	// CPU - UMC
	PEDIDO_LECTURA_VARIABLE, 		// CPU - UMC
	INDICAR_PID, 					// CPU - UMC
	PEDIDO_ESCRITURA,				// CPU - UMC
	FINALIZAR_PROGRAMA,				// Núcleo - UMC / UMC - SWAP
	LEER_PAGINA, 					// UMC - Swap
	DEVOLVER_VARIABLE, 				// UMC - CPU
	RESPUESTA_PEDIDO, 				// UMC - CPU
	ABORTO_PROCESO,					// CPU - Núcleo
	PCB_FIN_EJECUCION,				// CPU - Núcleo
	RESPUESTA_WAIT,					// Núcleo - CPU
	DEVOLVER_VAR_COMPARTIDA,		// Núcleo - CPU
	TAMANIO_STACK,					// Núcleo - CPU
	SENIAL_SIGUSR1,					// CPU - Núcleo
	// Mensajes de un struct de elementos:
	INICIAR_PROGRAMA, 				// Núcleo - UMC / UMC - Swap
	ENVIAR_SCRIPT, 					// Consola - Núcleo
	PCB, 							// Núcleo - CPU
	PCB_FIN_QUANTUM,				// CPU - Núcleo
	PCB_ENTRADA_SALIDA,				// CPU - Núcleo
	PCB_WAIT, 						// CPU - Núcleo
	OBTENER_VAR_COMPARTIDA,			// CPU - Núcleo
	IMPRIMIR_TEXTO, 				// CPU - Núcleo / Núcleo - Consola
	DEVOLVER_INSTRUCCION,			// UMC - CPU
	DEVOLVER_PAGINA,				// Swap - UMC
	WAIT_REQUEST,					// CPU - Núcleo
	SIGNAL_REQUEST,					// CPU - Núcleo
	ENTRADA_SALIDA,					// CPU - Núcleo
	GRABAR_VAR_COMPARTIDA,			// CPU - Núcleo
	ESCRIBIR_PAGINA,				// UMC - Swap

	FIN_DEL_PROTOCOLO
} protocolo;

/*** PROTOTIPOS ***/
// -- Funciones definitivas para enviar y recibir PAQUETES:
int aplicar_protocolo_enviar(int fdReceptor, int head, void * mensaje);
void* aplicar_protocolo_recibir(int fdEmisor, int* head);

// -- Calcula tamaño mensaje para serealización:
int calcularTamanioMensaje(int head, void* mensaje);

// -- Serealización y deserealización GENERAL:
void * serealizar(int head, void * mensaje, int tamanio);
void * deserealizar(int head, void * buffer, int tamanio);

// -- Serealizaciones y deserealizaciones PARTICULARES:
void* serealizarPcb(void* mensaje, int tamanio);
pcb* deserealizarPcb(void* buffer, int tamanio);
void* serealizarPedidoEscritura(void* mensaje, int tamanio);
solicitudEscritura* deserealizarPedidoEscritura(void* buffer, int tamanio);
void* serealizarInicioPrograma(void* mensaje, int tamanio);
inicioPrograma* deserealizarInicioPrograma(void* buffer, int tamanio);
void* serealizarTextoMasUnInt(void* mensaje, int tamanio);
pedidoIO* deserealizarTextoMasUnInt(void* buffer, int tamanio);
void*  serealizarTextoMasDosInt(void* mensaje, int tamanio);
inicioPrograma*  deserealizarTextoMasDosInt(void* buffer, int tamanio);
void* serealizarDosInt(void* mensaje, int tamanio);
solicitudLeerPagina* deserealizarDosInt(void* buffer, int tamanio);
void* serealizarTresInt(void* mensaje, int tamanio);
direccion* deserealizarTresInt(void* buffer, int tamanio);
void* serealizarEscribirPagina(void* mensaje, int tamanio);
solicitudEscribirPagina* deserealizarEscribirPagina(void* buffer, int tamanio);
void* serealizarDevolverPagina(void* mensaje, int tamanio);
paginaSwap* deserealizarDevolverPagina(void* buffer, int tamanio);

// -- Auxiliares:
int getStartInstruccion(t_intructions instruccion);
int getOffsetInstruccion (t_intructions instruccion);
t_intructions cargarIndiceCodigo(t_puntero_instruccion primera_instruccion, t_size offset_instruccion);
int calcularTamanioIndiceStack(pcb* unPcb);
int calcularTamanioPcb(pcb* mensaje);

#endif /* UTILIDADES_COMUNICACIONES_H_ */

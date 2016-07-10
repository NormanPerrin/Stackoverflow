#ifndef UTILIDADES_TIPOSDEDATOS_H_
#define UTILIDADES_TIPOSDEDATOS_H_

#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <parser/metadata_program.h>

// Dirección lógica:
typedef struct {
		int pagina, offset, size;
} __attribute__((packed)) direccion;

// Cadena de texto:
typedef struct {
		int tamanio;
		char * cadena;
} __attribute__((packed)) string;

// Solicitud de inicio de un programa:
typedef struct {
	int pid, paginas;
	char *contenido;
} __attribute__((packed)) inicioPrograma;

// Variable compartida AnSISOP:
typedef struct {
	char *nombre;
	int valor;
} __attribute__((packed)) var_compartida;

// Variable AnSISOP:
typedef struct {
	char *nombre;
	direccion direccion;
} __attribute__((packed)) variable;

// Pedido de Entrada/Salida:
typedef struct {
	int tiempo;
	char* nombreDispositivo;
} __attribute__((packed)) pedidoIO;

// Para indicar si se pudo alocar los segmentos de un programa:
typedef int respuestaInicioPrograma;

// Valores que puede tomar 'respuestaInicioPrograma':
typedef enum {
	CREADO, NO_CREADO
} estadoDelHeap;

// Elemento del Índice de Stack:
typedef struct {
	direccion* args;
	t_dictionary* vars;
	int retPos;
	direccion retVar;
} registroStack;

// PCB de un proceso:
typedef struct pcb{
	int pid;
	int paginas_codigo; // tamaño en páginas del segmento de código
	int paginas_stack; // tamaño en páginas del segmento de stack
	int pc; // program counter
	int id_cpu; // id del CPU que ejecuta al proceso actualmente
	int cantidad_instrucciones;
	int quantum, quantum_sleep;

	int ultimaPosicionIndiceStack;
	int stackPointer;
	int paginaActualCodigo;
	int paginaActualStack;
	int primerPaginaStack;
	int numeroContextoEjecucionActualStack;

	int tamanioIndiceCodigo, tamanioIndiceStack, tamanioIndiceEtiquetas; // Tamaños en bytes de los índices
	t_intructions* indiceCodigo;
	char* indiceEtiquetas;
	t_list* indiceStack;
} __attribute__((packed)) pcb;

// Pedido de Lectura de CPU a UMC (dirección lógica):
typedef struct {
	int pagina, offset, tamanio;
} __attribute__((packed)) solicitudLectura;

// Pedido de Escritura de CPU a UMC (dirección lógica + variable):
typedef struct {
	int pagina, offset, tamanio, contenido;
} __attribute__((packed)) solicitudEscritura;

// Valores que puede tomar una respuesta ante un pedido a UMC:
typedef enum{
	PERMITIDO, NO_PERMITIDO
} estadoPedido;

// Respuesta a un pedido de Lectura de Instrucción:
typedef struct {
	int pagina;
	char *contenido;
} __attribute__((packed))devolverPaginaInstruccion;

// Respuesta a un pedido de Lectura de Variable:
typedef struct {
	int pagina, contenido;
} __attribute__((packed))devolverPaginaVariable;

// Pedido de Lectura de UMC a Swap:
typedef struct {
	int pid, pagina;
} __attribute__((packed))solicitudLeerPagina;

// Pedido de Escritura de UMC a Swap (contenido = variable a escribir):
typedef struct {
	int pid, pagina, contenido;
} __attribute__((packed))solicitudEscribirPagina;

#endif /* UTILIDADES_TIPOSDEDATOS_H_ */

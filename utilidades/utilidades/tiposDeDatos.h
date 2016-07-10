#ifndef UTILIDADES_TIPOSDEDATOS_H_
#define UTILIDADES_TIPOSDEDATOS_H_

#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <parser/metadata_program.h>

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

typedef struct {
	int pid, paginas;
	char *contenido;
} __attribute__((packed)) inicioPrograma;

typedef struct {
	char *nombre;
	int valor;
} __attribute__((packed)) var_compartida;

typedef struct {
	char *nombre;
	direccion direccion;
} __attribute__((packed)) variable;

typedef struct {
	int tiempo;
	char* nombreDispositivo;
} __attribute__((packed)) pedidoIO;

typedef int respuestaInicioPrograma;

typedef enum {
	CREADO, NO_CREADO
} estadoDelHeap;

typedef struct {
	direccion* args;
	t_dictionary* vars;
	int retPos;
	direccion retVar;
} registroStack;

typedef struct pcb{
	int pid; // id del proceso
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
	t_intructions* indiceCodigo;  // Índice de código
	char* indiceEtiquetas;  // Índice de etiquetas
	t_list* indiceStack; // Índice de stack
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

#endif /* UTILIDADES_TIPOSDEDATOS_H_ */

#ifndef UTILIDADES_TIPOSDEDATOS_H_
#define UTILIDADES_TIPOSDEDATOS_H_

#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
//#include <parser/parser.h>
#include <parser/metadata_program.h>

typedef struct {
	int tamanio;
	char* cadena;
} __attribute__((packed)) string;

// Dirección lógica:
typedef struct {
	int pagina, offset, size;
} __attribute__((packed)) direccion;

// Solicitud de inicio de un programa:
typedef struct {
	int pid, paginas;
	string contenido;
} __attribute__((packed)) inicioPrograma;

// Variable compartida AnSISOP:
typedef struct {
	int valor;
	char *nombre;
} __attribute__((packed)) var_compartida;

// Variable AnSISOP:
typedef struct {
	char nombre; // char 'id' --> 1 bytes
	direccion direccion; // --> 12 bytes
} __attribute__((packed)) variable; // o argumento, son lo mismo

// Pedido de Entrada/Salida:
typedef struct {
	int tiempo;
	char* nombreDispositivo;
} __attribute__((packed)) pedidoIO;

// Respuesta ante el inicio de un programa de UMC a Núcleo:
	/* MANDA DIRECTAMENTE UN INT --> RESPUESTA_PEDIDO: PERMITIDO ó NO_PERMITIDO */

// Elemento del Índice de Stack:
typedef struct {
	int cantidad_args;
	t_list* args; // valores -> nombre + dirección
	int cantidad_vars;
	t_list* vars; // valores -> nombre + dirección
	int retPos;
	direccion retVar;
}  __attribute__((packed)) registroStack;

// PCB de un proceso:
typedef struct pcb{
	int cantidad_instrucciones, id_cpu, paginaActualCodigo, paginaActualStack;
	int paginas_codigo; // tamaño en páginas del segmento de código
	int paginas_stack; // tamaño en páginas del segmento de stack
	int pc; // program counter
	int pid, primerPaginaStack, quantum, quantum_sleep, stackPointer;
	int tamanioIndiceCodigo, tamanioIndiceEtiquetas; // Tamaños en bytes de los índices
	int cantidad_registros_stack; // Cantidad de elementos en el índice de stack
	t_intructions* indiceCodigo;
	t_list* indiceStack; // valores -> registroStack
	char* indiceEtiquetas;
} __attribute__((packed)) pcb;

// Pedido de Lectura de CPU a UMC (dirección lógica):
typedef struct {
	int pagina, offset, tamanio;
} __attribute__((packed)) solicitudLectura;

// Pedido de Escritura de CPU a UMC (dirección lógica + variable):
typedef struct {
	int pagina, offset, contenido;
} __attribute__((packed)) solicitudEscritura;

// Valores que puede tomar la respuesta de UMC ante un pedido:
typedef enum{
	PERMITIDO = 1, NO_PERMITIDO
}  __attribute__((packed)) estadoPedido;

// Valores que puede tomar la respuesta de Núcleo a Consola al iniciar programa:
typedef enum{
	ACEPTADO = 1, RECHAZADO, ERROR_CONEXION
}  __attribute__((packed)) estadoInicio;

// Respuesta de Swap a un pedido de Lectura de UMC: /* MANDA UN CHAR* */

// Respuesta a un pedido de Lectura de Variable de UMC a CPU: /* MANDA UN INT */

// Respuesta a un pedido de Lectura de Instrucción de UMC a CPU: /* MANDA UN CHAR* */

// Pedido de Lectura de UMC a Swap:
typedef struct {
	int pid, pagina;
} __attribute__((packed))solicitudLeerPagina;

// Pedido de Escritura de UMC a Swap (contenido = variable a escribir):
typedef struct {
	int pid, pagina;
	char* contenido;
} __attribute__((packed))solicitudEscribirPagina;

#endif /* UTILIDADES_TIPOSDEDATOS_H_ */

#ifndef LIB_FCPU_H_
#define LIB_FCPU_H_

#include <utilidades/general.h>
#include <utilidades/comunicaciones.h>
#include <commons/config.h>
#include <commons/log.h>
#include <parser/parser.h>
#include <parser/metadata_program.h>

#define PACKAGESIZE 1024 // Size máximo de paquete para sockets
#define RUTA_CONFIG_CPU "configCPU.txt"

// Estructuras
typedef struct {
	int puertoNucleo; // Puerto donde se encuentra escuchando el proceso Núcleo
	char *ipNucleo; // IP del proceso Núcleo
	int puertoUMC;
	char *ipUMC;
} t_configuracion;

// Globales
t_configuracion *config;
int fd_clienteNucleo, fd_clienteUMC;
int tamanioPagina;

// Cabeceras
void conectarConNucleo();
void conectarConUMC();
pcb * recibirPCB(void * mensaje);
/*void esperar_ejecucion(); // Espera PCB para ejecutar de Núcleo idefinidamente*/
void setearValores_config(t_config * archivoConfig);
void liberarEstructura(); // Libera la memoria reservada en setear config
int validar_servidor(char *id); // Valida si la conexión es UMC o Nucleo
int validar_cliente(char *id); // para que no joda con error

// Prototipos de Primitivas AnSISOP
t_puntero definirVariable(t_nombre_variable identificador_variable);
t_puntero obtenerPosicionVariable(t_nombre_variable identificador_variable);
t_valor_variable dereferenciar(t_puntero direccion_variable);
void asignar(t_puntero direccion_variable, t_valor_variable valor);
t_valor_variable obtenerValorCompartida(t_nombre_compartida variable);
t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor);
void irAlLabel(t_nombre_etiqueta t_nombre_etiqueta);
void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar);
void retornar(t_valor_variable retorno);
void imprimirAnSISOP(t_valor_variable valor_mostrar);
void imprimirTexto(char* texto);
void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo);
void wait(t_nombre_semaforo identificador_semaforo);
void signal(t_nombre_semaforo identificador_semaforo);

// Estructuras funcionesAnSISOP
AnSISOP_funciones funcionesAnSISOP = {
		.AnSISOP_definirVariable			= definirVariable,
		.AnSISOP_obtenerPosicionVariable	= obtenerPosicionVariable,
		.AnSISOP_dereferenciar				= dereferenciar,
		.AnSISOP_asignar					= asignar,
		.AnSISOP_obtenerValorCompartida		= obtenerValorCompartida,
		.AnSISOP_asignarValorCompartida		= asignarValorCompartida,
		.AnSISOP_irAlLabel					= irAlLabel,
		.AnSISOP_llamarConRetorno			= llamarConRetorno,
		.AnSISOP_retornar					= retornar,
		.AnSISOP_imprimir					= imprimirAnSISOP,
		.AnSISOP_imprimirTexto				= imprimirTexto,
};
AnSISOP_kernel funcionesKernel = {
		.AnSISOP_wait		= wait,
		.AnSISOP_signal 	= signal,
};


#endif /* LIB_FCPU_H_ */

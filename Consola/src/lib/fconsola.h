#ifndef LIB_FCONSOLA_H_
#define LIB_FCONSOLA_H_

#include <utilidades/general.h>
#include <utilidades/comunicaciones.h>
#include <utilidades/sockets.h>
#include <commons/config.h>
#include <commons/log.h>
#include <sys/stat.h>
#define PACKAGESIZE 1024 // Size máximo de paquete para sockets

#define RUTA_CONFIG_CONSOLA "/home/utnso/tp-2016-1c-Cazadores-de-cucos/Consola/configConsola.txt"

// Variables globales
int puertoNucleo;
char * ipNucleo;
int fd_nucleo; // Socket Consola - Núcleo
char * rutaScript;
string* programa;
t_log * logger;

// Cabeceras
void validar_argumentos(int arg); // Valida argumentos de entrada
void leerScript(char * rutaScript);
void setearValores_config(t_config * archivoConfig);
void liberarRecursos(); // Libera memoria asignada
void conectarConNucleo();
int validar_servidor(char *id); // Valida si la conexión es de Núcleo
int validar_cliente(char *id); // Es para poner su definición y que no jodan errores
void esperar_mensajes(); // Espera mensajes de Núcleo: Fin, Imprimir, Imprimir_texto
void crearLogger();

#endif /* LIB_FCONSOLA_H_ */

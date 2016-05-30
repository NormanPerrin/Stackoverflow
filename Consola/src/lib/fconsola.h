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

//#define RUTA_CONFIG_CONSOLA "configConsola.txt"

// Variables globales
int puertoNucleo; // Puerto donde se encuentra escuchando el proceso Núcleo
char * ipNucleo; // IP del proceso Núcleo
int fd_nucleo; // Socket Consola - Núcleo
char * rutaScript;
t_string* programa;

// Estructuras

// Cabeceras
void validar_argumentos(int arg); // Valida argumentos de entrada
void leerScript(char * rutaScript);
void testLecturaArchivoDeConfiguracion();
void setearValores_config(t_config * archivoConfig);
void liberarRecusos(); // Libera memoria asignada
void conectarConNucleo();
int validar_servidor(char *id); // Valida si la conexión es de Núcleo
int validar_cliente(char *id); // Es para poner su definición y que no jodan errores
void esperar_mensajes(); // Espera mensajes de Núcleo: Fin, Imprimir, Imprimir_texto
void imprimir(); // Imprime por consola

#endif /* LIB_FCONSOLA_H_ */

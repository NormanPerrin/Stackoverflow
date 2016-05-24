#ifndef LIB_FCONSOLA_H_
#define LIB_FCONSOLA_H_

#include <utilidades/general.h>
#include <utilidades/comunicaciones.h>
#include <utilidades/sockets.h>
#include <commons/config.h>
#include <commons/log.h>
#define PACKAGESIZE 1024 // Size máximo de paquete para sockets
#define RUTA_CONFIG_CONSOLA "configConsola.txt"
#define ENVIAR_SCRIPT 7

// Variables globales
int puertoNucleo; // Puerto donde se encuentra escuchando el proceso Núcleo
char * ipNucleo; // IP del proceso Núcleo
int fd_nucleo; // Socket consola - nucleo
t_config config;
t_string * nombreScript;

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
void enviar_script(char *ruta); // Envía ruta de script AnSISOP a Núcleo
void esperar_mensajes(); // Espera mensajes de Núcleo: Fin, Imprimir, Imprimir_texto
void imprimir(); // Imprime por consola

#endif /* LIB_FCONSOLA_H_ */

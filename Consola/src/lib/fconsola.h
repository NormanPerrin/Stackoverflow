#ifndef LIB_FCONSOLA_H_
#define LIB_FCONSOLA_H_

#include <utilidades/comunicaciones.h>
#include <sys/stat.h>

#define PACKAGESIZE 1024 // Size máximo de paquete para sockets
#define RUTA_CONFIG_CONSOLA "configConsola.txt"

// Variables globales
int puertoNucleo;
char * ipNucleo;
int fd_nucleo; // Socket Consola - Núcleo
char * rutaScript;
char* programa; // Script AnSISOP asociado
int tamanioPrograma;
t_log * logger;

// Cabeceras
void crearLoggerConsola();
void validar_argumentos(int arg); // Valida argumentos de entrada
void leerScript(char * rutaScript);
void setearValores_config(t_config * archivoConfig);
void exitConsola();
void liberarRecursos(); // Libera memoria asignada
void conectarCon_Nucleo();
int validar_servidor(char *id); // Valida si la conexión es de Núcleo
int validar_cliente(char *id); // Es para poner su definición y que no jodan errores

#endif /* LIB_FCONSOLA_H_ */

#ifndef LIB_FCONSOLA_H_
#define LIB_FCONSOLA_H_

#include <utilidades/general.h>
#include <commons/config.h>
#include <commons/log.h>
#define PACKAGESIZE 1024 // Size máximo de paquete para sockets
#define RUTA_CONFIG_CONSOLA "configConsola.txt"

// Variables globales
int puertoNucleo; // Puerto donde se encuentra escuchando el proceso Núcleo
char * ipNucleo; // IP del proceso Núcleo

// Estructuras

// Cabeceras
void conectarConNucleo();
void setearValores_config(t_config * archivoConfig);
void testLecturaArchivoDeConfiguracion();
void liberarRecusos(); // Libera memoria asignada
int validar_servidor(char *id); // Valida si la conexión es de Núcleo
int validar_cliente(char *id); // Es para poner su definición y que no jodan errores

#endif /* LIB_FCONSOLA_H_ */

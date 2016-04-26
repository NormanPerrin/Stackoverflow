#ifndef LIB_FNUCLEO_H_
#define LIB_FNUCLEO_H_

#include <utilidades/sockets.h>
#include <utilidades/general.h>
#include <commons/config.h>

//Variables Globales

int puerto_Nucleo;
int puertoPrograma;
int puertoCPU;
int quantum;
int retardoQuantum;

// Tengo que ver cómo hago con estos arrays
/*char semaforosID[][];
int semaforosValInicial[];
char ioID[][];
int retardosIO[];
char variablesCompartidas[][];*/

// Estructuras

// Cabeceras

void escucharCPU(); // Conexión con CPUs
void escucharConsola(); // Conexión con Consolas

void abrirArchivoDeConfiguracion(char *ruta);
void setearValores_config(t_config * archivoConfig);

#endif /* LIB_FNUCLEO_H_ */

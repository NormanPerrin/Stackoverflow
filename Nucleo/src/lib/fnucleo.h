#ifndef LIB_FNUCLEO_H_
#define LIB_FNUCLEO_H_

#include <utilidades/sockets.h>
#include <utilidades/general.h>
#include <commons/config.h>
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

// Variables Globales

int puerto_Nucleo;
int puertoPrograma;
int puertoCPU;
int quantum;
int retardoQuantum;
// arrays:
char** semaforosID;
int* semaforosValInicial;
char** ioID;
int* retardosIO;
char** variablesCompartidas;

// Estructuras

// Cabeceras

void escucharCPU(); // Conexión con CPUs
void escucharConsola(); // Conexión con Consolas
void abrirArchivoDeConfiguracion(char *ruta);
void setearValores_config(t_config * archivoConfig);
void pasarCadenasArray(char** cadenas, char** variablesConfig);
void pasarEnterosArray(int* numeros, char** variablesConfig);

#endif /* LIB_FNUCLEO_H_ */

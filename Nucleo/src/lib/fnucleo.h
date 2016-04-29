#ifndef LIB_FNUCLEO_H_
#define LIB_FNUCLEO_H_

#include <utilidades/sockets.h>
#include <utilidades/general.h>
#include <commons/config.h>
#include <commons/log.h>
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))
#define CONEXIONES_PERMITIDAS 10
#define PACKAGESIZE 1024 // Size máximo de paquete para sockets

// Variables Globales
int puertoPrograma;
int puertoCPU;
int puertoUMC; // Puerto donde se encuentra escuchando el proceso UMC (no viene de archivo de config)
const char * ipUMC = "127.0.0.1"; // IP del proceso UMC
int quantum;
int retardoQuantum;
// --Arrays
char** semaforosID;
int* semaforosValInicial;
char** ioID;
int* retardosIO;
char** variablesCompartidas;
t_log* logger;

// Estructuras

// Cabeceras
void conectarConUMC();
void escucharACPU(); // Conexión con CPUs
void escucharAConsola(); // Conexión con Consolas
void setearValores_config(t_config * archivoConfig);
void crearLogger();
// --Funciones MUY auxiliares
void imprimirCadenas(char** cadenas);
void imprimirNumeros(int* numeros);
void pasarCadenasArray(char** cadenas, char** variablesConfig);
void pasarEnterosArray(int* numeros, char** variablesConfig);

// Tests (PROVISORIOS)
void testLecturaArchivoDeConfiguracion(); // Imprimo todas las variables, para ver si se setearon bien

#endif /* LIB_FNUCLEO_H_ */

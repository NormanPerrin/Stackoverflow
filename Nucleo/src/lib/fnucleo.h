#ifndef LIB_FNUCLEO_H_
#define LIB_FNUCLEO_H_

#include <utilidades/sockets.h>
#include <utilidades/general.h>
#include <pthread.h>
#include <commons/config.h>
#include <commons/log.h>
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))
#define CONEXIONES_PERMITIDAS 10
#define PACKAGESIZE 1024 // Size máximo de paquete para sockets
#define RUTA_CONFIG_NUCLEO "configNucleo.txt" // Ruta archivo config

// Estructuras
typedef struct {
		int puertoPrograma;
		int puertoCPU;
		int puertoUMC;
		char *ipUMC;
		int quantum;
		int retardoQuantum;
		char** semaforosID;
		int* semaforosValInicial;
		char** ioID;
		int* retardosIO;
		char** variablesCompartidas;
		int cantidadPaginasStack;
	} t_configuracion;

// Variables Globales
t_configuracion *config;
int fd_serverUMC; // cliente de UMC
t_log* logger;

// Estructuras


// Cabeceras
void conectarConUMC();
void crear_hilos_conexion(); // Crea 2 hilos: 1 hilo escuchar_conexiones(CPU) y otro escuchar_conexiones(Consola);
void escuchar_conexiones(); // Escucha conexiones CPU y Consola
void escucharACPU();
void escucharAConsola();
void setearValores_config(t_config * archivoConfig);
void crearLogger();
int validar_cliente(char *id); // Valida que el cliente sea CPU o Consola
int validar_servidor(char *id); // Valida que el servidor sea UMC
// --Funciones auxiliares--
int* pasarArrayDeStringsANumeros(char** vectorStrings);
void pasarCadenasArray(char** cadenas, char** variablesConfig);
void pasarEnterosArray(int* numeros, char** variablesConfig);

#endif /* LIB_FNUCLEO_H_ */

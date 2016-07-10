#ifndef LIB_GLOBALESCPU_H_
#define LIB_GLOBALESCPU_H_

#include <utilidades/comunicaciones.h>

#define PACKAGESIZE 1024 // Size máximo de paquete para sockets
#define RUTA_CONFIG_CPU "configCPU.txt"

// Estructuras:
typedef struct {
	int puertoNucleo; // Puerto donde se encuentra escuchando el proceso Núcleo
	char *ipNucleo; // IP del proceso Núcleo
	int puertoUMC;
	char *ipUMC;
} t_configuracion;

info_quantum * infoQuantum;

// Variables Globales:
t_configuracion *config;
int fdNucleo, fdUMC, tamanioPagina;
t_log * logger;
pcb * pcbActual;

#endif /* LIB_GLOBALESCPU_H_ */

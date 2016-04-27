#ifndef LIB_FUMC_H_
#define LIB_FUMC_H_

#include <utilidades/sockets.h>
#include <utilidades/general.h>
#include <commons/config.h>
#include <commons/log.h>
#define CONEXIONES_PERMITIDAS 10
#define PACKAGESIZE 1024 // Size máximo de paquete para sockets

// Variables globales
int puerto; // Puerto para recibir conexiones de Núcleo y CPUs
const char * ipSwap; // IP del Swap
int puertoSwap; // Puerto donde se encuentra escuchando el proceso Swap

// Estructuras

// Cabeceras
void setearValores_config(t_config * archivoConfig);
void conectarConSwap();
void escucharANucleo();
void escucharACPU();
void esperarPaqueteDelCliente(int fd_escucha, int fd_nuevoCliente);

#endif /* LIB_FUMC_H_ */

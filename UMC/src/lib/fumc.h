#ifndef LIB_FUMC_H_
#define LIB_FUMC_H_

#include <utilidades/sockets.h>
#include <utilidades/general.h>
#include <commons/config.h>
#include <commons/log.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define CONEXIONES_PERMITIDAS 10
#define PACKAGESIZE 1024 // Size máximo de paquete para sockets

// Estructuras
typedef struct {
	int backlog;
	int puerto; // para conexiones de CPU y Núcleo
	char *ip_swap;
	int puerto_swap;
	int marcos;
	int marco_size;
	int marco_x_proceso;
	int entradas_tlb;
	int retardo;
} t_configuracion;

// Cabeceras
void abrirArchivoConfig();
void conectarConSwap();
void escucharANucleo();
void escucharACPU();
void esperarPaqueteDelCliente(int fd_escucha, int fd_nuevoCliente);
/* void crearHilos(); // Más adelante para implementar con hilos
void establecerConexionSwap();
void servidor();
void consola();*/

#endif /* LIB_FUMC_H_ */

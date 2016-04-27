#ifndef LIB_FUMC_H_
#define LIB_FUMC_H_


	#include <commons/config.h>
	#include <pthread.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <utilidades/general.h>
	#include <utilidades/sockets.h>


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
	void abrirArchivoConfig(char *ruta);
	void crearHilos();
	void servidor();
	void consola();
	void establecerConexionSwap();

#endif /* LIB_FUMC_H_ */

#ifndef LIB_FUMC_H_
#define LIB_FUMC_H_

#include <commons/config.h>
#include <utilidades/general.h>

// Estructuras

typedef struct {
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
t_configuracion* abrirArchivoConfig(char *ruta);

#endif /* LIB_FUMC_H_ */

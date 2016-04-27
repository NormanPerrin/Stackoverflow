#ifndef LIB_FUMC_H_
#define LIB_FUMC_H_

#include <utilidades/sockets.h>
#include <utilidades/general.h>
#include <commons/config.h>
#include <commons/log.h>
#define CONEXIONES_PERMITIDAS 10
#define PACKAGESIZE 1024 // Size máximo de paquete para sockets

// Estructuras

// Cabeceras
void setearValores_config(t_config * archivoConfig);

#endif /* LIB_FUMC_H_ */

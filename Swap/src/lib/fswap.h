#ifndef FSWAP_H_
#define FSWAP_H_

#include <utilidades/sockets.h>
#include <utilidades/general.h>
#include <commons/config.h>
#include <commons/log.h>
#define CONEXIONES_PERMITIDAS 10
#define PACKAGESIZE 1024 // Size máximo de paquete para sockets

// Variables globales
int puertoEscuchaUMC;

// Estructuras

// Cabeceras
void setearValores_config(t_config * archivoConfig);
void escucharAUMC();

#endif /* FSWAP_H_ */

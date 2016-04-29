#ifndef FSWAP_H_
#define FSWAP_H_

	#include <utilidades/sockets.h>
	#include <utilidades/general.h>
	#include <commons/config.h>
	#include <commons/log.h>

	#define CONEXIONES_PERMITIDAS 10
	#define PACKAGESIZE 1024 // Size máximo de paquete para sockets
	#define RUTA_CONFIG_SWAP "/home/utnso/Escritorio/projects/tp-2016-1c-Cazadores-de-cucos/Swap/config.txt"

	// Estructuras
	typedef struct {
		int puerto;
		char *nombreSwap;
		int cantidadPaginas;
		int tamanioPagina;
		int retardoCompactacion;
	} t_configuracion;

	// Cabeceras
	void setearValores_config(t_config * archivoConfig);
	void escucharUMC();
	void liberarEstructuraConfig();

#endif /* FSWAP_H_ */

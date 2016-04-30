#ifndef FSWAP_H_
#define FSWAP_H_

	#include <utilidades/sockets.h>
	#include <utilidades/general.h>
	#include <commons/config.h>
	#include <commons/log.h>

	#define PACKAGESIZE 1024 // Size máximo de paquete para sockets

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
	void validarArgumentos(int argc, char **argv);


#endif /* FSWAP_H_ */

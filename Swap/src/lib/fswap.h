#ifndef FSWAP_H_
#define FSWAP_H_

	#include <utilidades/sockets.h>
	#include <utilidades/general.h>
	#include <commons/config.h>
	#include <commons/log.h>

	#define PACKAGESIZE 1024 // Size máximo de paquete para sockets
	#define RUTA_CONFIG_SWAP "configSwap.txt"

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
	void liberarEstructura(); // Libera memoria asignada en setear config
	void liberarRecusos();
	int validar_cliente(char *id); // Verifica que sea cliente UMC
	int validar_servidor(char *id); // Para que no joda con error


#endif /* FSWAP_H_ */

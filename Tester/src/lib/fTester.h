#ifndef FSWAP_H_
#define FSWAP_H_

	#include <utilidades/sockets.h>
	#include <utilidades/general.h>
	#include <commons/config.h>
	#include <commons/log.h>
    #include <utilidades/comunicaciones.h>

	// Cabeceras
	void mostrar_protocolos();
	void validar_argumentos(int argc);
	void testear(char* arg_puerto, char* arg_modo);
	int validar_cliente(char *id);
	int validar_servidor(char *id);
	void setearValores_config(t_config * archivoConfig);

#endif /* FSWAP_H_ */

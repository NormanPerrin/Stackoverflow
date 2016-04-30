#include "fconsola.h"

// Funciones

void setearValores_config(t_config * archivoConfig){
	puertoNucleo = config_get_int_value(archivoConfig, "PUERTO_NUCLEO");
	ipNucleo = strdup(config_get_string_value(archivoConfig, "IP_NUCLEO"));
}

void conectarConNucleo(){
int fd_serverConsola;

	fd_serverConsola = nuevoSocket();
	conectarSocket(fd_serverConsola, ipNucleo, puertoNucleo);
	handshake_cliente(fd_serverConsola, "C");
}

void testLecturaArchivoDeConfiguracion(){
	printf("Puerto Núcleo: %d\n", puertoNucleo);
	printf("IP Núcleo: %s\n", ipNucleo);
}

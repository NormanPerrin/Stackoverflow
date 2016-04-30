#include "fcpu.h"

// Funciones

void setearValores_config(t_config * archivoConfig){

	ipNucleo = strdup(config_get_string_value(archivoConfig, "IP_NUCLEO"));
	ipUMC = strdup(config_get_string_value(archivoConfig, "IP_UMC"));
	puertoNucleo = config_get_int_value(archivoConfig, "PUERTO_NUCLEO");
	puertoUMC = config_get_int_value(archivoConfig, "PUERTO_UMC");
}

void conectarConNucleo(){
int fd_serverCPU;

	fd_serverCPU = nuevoSocket();
	conectarSocket(fd_serverCPU, ipNucleo, puertoNucleo);
	handshake_cliente(fd_serverCPU, "C");
} // Soy cliente del Núcleo, es  decir, soy el que inicia la conexión con él

void conectarConUMC(){
int fd_serverCPU;

	fd_serverCPU = nuevoSocket();
	conectarSocket(fd_serverCPU, ipUMC, puertoUMC);
	handshake_cliente(fd_serverCPU, "C");
} // Soy cliente de la UMC, es  decir, soy el que inicia la conexión con ella

void testLecturaArchivoDeConfiguracion(){
	printf("Puerto Núcleo: %d\n", puertoNucleo);
	printf("IP Núcleo: %s\n", ipNucleo);
	printf("Puerto UMC: %d\n", puertoUMC);
	printf("IP UMC: %s\n", ipUMC);
}

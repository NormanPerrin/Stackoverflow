#include "fcpu.h"

// Funciones

void setearValores_config(t_config * archivoConfig){

	ipNucleo = strdup(config_get_string_value(archivoConfig, "IP_NUCLEO"));
	ipUMC = strdup(config_get_string_value(archivoConfig, "IP_UMC"));
	puertoNucleo = config_get_int_value(archivoConfig, "PUERTO_NUCLEO");
	puertoUMC = config_get_int_value(archivoConfig, "PUERTO_UMC");
}

void conectarConNucleo() {
	int fd_serverCPU;
	fd_serverCPU = nuevoSocket();
	conectarSocket(fd_serverCPU, ipNucleo, puertoNucleo);
	handshake_cliente(fd_serverCPU, "P");
}

void conectarConUMC(){
	int fd_serverCPU;
	fd_serverCPU = nuevoSocket();
	conectarSocket(fd_serverCPU, ipUMC, puertoUMC);
	handshake_cliente(fd_serverCPU, "P");
}

void testLecturaArchivoDeConfiguracion(){
	printf("Puerto Núcleo: %d\n", puertoNucleo);
	printf("IP Núcleo: %s\n", ipNucleo);
	printf("Puerto UMC: %d\n", puertoUMC);
	printf("IP UMC: %s\n", ipUMC);
}

void validarArgumentos(int argc, char **argv) {
	if(argc != 2) {
		printf("Ingrese el archivo de configuración como argumento\n");
		exit(-1);
	}
}

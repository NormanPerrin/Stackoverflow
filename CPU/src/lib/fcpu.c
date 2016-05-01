#include "fcpu.h"

// Globales
t_configuracion *config;

// Funciones
void setearValores_config(t_config * archivoConfig){
	config = (t_configuracion*)reservarMemoria(sizeof(t_configuracion));
	config->ipNucleo = strdup(config_get_string_value(archivoConfig, "IP_NUCLEO"));
	config->ipUMC = strdup(config_get_string_value(archivoConfig, "IP_UMC"));
	config->puertoNucleo = config_get_int_value(archivoConfig, "PUERTO_NUCLEO");
	config->puertoUMC = config_get_int_value(archivoConfig, "PUERTO_UMC");
}

void conectarConNucleo() {
	int fd_serverCPU;
	fd_serverCPU = nuevoSocket();
	int ret = conectarSocket(fd_serverCPU, config->ipNucleo, config->puertoNucleo);
	validar_conexion(ret, 1); // Es terminante por ser cliente
	handshake_cliente(fd_serverCPU, "P");
}

void conectarConUMC(){
	int fd_serverCPU;
	fd_serverCPU = nuevoSocket();
	int ret = conectarSocket(fd_serverCPU, config->ipUMC, config->puertoUMC);
	validar_conexion(ret, 1);
	handshake_cliente(fd_serverCPU, "P");
}

void liberarEstructura() {
	free(config->ipNucleo);
	free(config->ipUMC);
	free(config);
}

int validar_servidor(char *id) {
	if( !strcmp(id, "U") || !strcmp(id, "N") ) {
		printf("Servidor aceptado\n");
		return TRUE;
	} else {
		printf("Servidor rechazado\n");
		return FALSE;
	}
}
int validar_cliente(char *id) {return 0;}

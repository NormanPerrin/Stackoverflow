#include "fcpu.h"


// Funciones
void setearValores_config(t_config * archivoConfig){
	config = (t_configuracion*)reservarMemoria(sizeof(t_configuracion));
	config->ipNucleo = strdup(config_get_string_value(archivoConfig, "IP_NUCLEO"));
	config->ipUMC = strdup(config_get_string_value(archivoConfig, "IP_UMC"));
	config->puertoNucleo = config_get_int_value(archivoConfig, "PUERTO_NUCLEO");
	config->puertoUMC = config_get_int_value(archivoConfig, "PUERTO_UMC");
}

void conectarConNucleo() {
	fd_serverCPU = nuevoSocket();
	int ret = conectarSocket(fd_serverCPU, config->ipNucleo, config->puertoNucleo);
	validar_conexion(ret, 1); // Es terminante por ser cliente
	handshake_cliente(fd_serverCPU, "P");
}

void conectarConUMC(){
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


void esperar_ejecucion() {

	uint8_t *head = (uint8_t*)reservarMemoria(1); // 0 .. 255

	while(TRUE) {
		int ret;
		ret = recibirPorSocket(fd_serverCPU, head, 1);
		validar_recive(ret, 1); // es terminante ya que si hay un error en el recive o desconexión debe terminar
		aplicar_protocolo_recibir(fd_serverCPU, *head);
	}

	free(head);
}

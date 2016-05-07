#include "fconsola.h"


void validar_argumentos(int arg) {
	if(arg != 2) {
		printf("Debe ingresar el archivo a ejecutar como parámetro\n");
		exit(1);
	}
}

void setearValores_config(t_config * archivoConfig){
	puertoNucleo = config_get_int_value(archivoConfig, "PUERTO_NUCLEO");
	ipNucleo = strdup(config_get_string_value(archivoConfig, "IP_NUCLEO"));
}

void conectarConNucleo(){
	fd_serverConsola = nuevoSocket();
	int ret = conectarSocket(fd_serverConsola, ipNucleo, puertoNucleo);
	validar_conexion(ret, 1); // Al ser cliente es terminante
	handshake_cliente(fd_serverConsola, "C");
}

void testLecturaArchivoDeConfiguracion(){
	printf("Puerto Núcleo: %d\n", puertoNucleo);
	printf("IP Núcleo: %s\n", ipNucleo);
}

void liberarRecusos() {
	free(ipNucleo);
}

int validar_servidor(char *id) {
	if(!strcmp(id, "N")) {
		printf("Servidor aceptado\n");
		return TRUE;
	} else {
		printf("Servidor rechazado\n");
		return FALSE;
	}
}
int validar_cliente(char *id) {return 0;}

void enviar_script(char *ruta) {
	enviarPorSocket(fd_serverConsola, ruta, sizeof(ruta));
}

void esperar_mensajes() {
	while(TRUE) {
		int head; // TODO arreglar protocolo
		recibirPorSocket(fd_serverConsola, &head, INT);
	}
}

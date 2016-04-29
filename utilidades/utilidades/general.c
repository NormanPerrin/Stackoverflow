#include "general.h"
#include <stdlib.h>
#include <stdio.h>

void* reservarMemoria(int size) {
	void* puntero = malloc(size);
	if(puntero == NULL) {
		fprintf(stderr, "Error al reservar memoria\n");
		exit(ERROR);
	}
	return puntero;
}


void leerArchivoDeConfiguracion(char * ruta) {
	t_config * archivoConfig;

	if (comprobarQueExistaArchivo(ruta) == ERROR){
		manejarError("Error: Archivo de configuración no encontrado\n");
	}else{

	archivoConfig = config_create(ruta);
	setearValores_config(archivoConfig); // Redefinido en cada proceso (Ejemplo en Núcleo)
	config_destroy(archivoConfig); // Libero la estructura archivoConfig

	printf("El archivo de configuración ha sido leído correctamente\n");
	}
}

int comprobarQueExistaArchivo(char * ruta) {
	if( access(ruta, F_OK ) != ERROR ) {
	    return TRUE;
	} else {
	    return ERROR;
	}
}

void handshake_servidor(int sockCliente, char *mensaje) {

	enviarPorSocket(sockCliente, mensaje, CHAR*2);

	char *buff = (char*)reservarMemoria(CHAR*2);

	recibirPorSocket(sockCliente, buff, CHAR*2);

	buff[1] = '\0';
	printf("Handshake: %s\n", buff);

	free(buff);
}

void handshake_cliente(int sockClienteDe, char *mensaje) {

	char *buff = (char*)reservarMemoria(CHAR*2);
	recibirPorSocket(sockClienteDe, buff, CHAR*2);
	buff[1] = '\0';
	printf("Handshake: %s\n", buff);

	free(buff);
	enviarPorSocket(sockClienteDe, mensaje, CHAR*2);
}

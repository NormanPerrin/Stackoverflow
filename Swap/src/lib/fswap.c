#include "fswap.h"

// Globales
t_configuracion *config;
int sockUMC;

// Funciones
void setearValores_config(t_config * archivoConfig){
	config = (t_configuracion*)reservarMemoria(sizeof(t_configuracion));
	config->nombreSwap = (char*)reservarMemoria(CHAR*30);
	char *nombre_temp;

	config->puerto = config_get_int_value(archivoConfig, "Puerto_Escucha");
	nombre_temp = config_get_string_value (archivoConfig, "Nombre_Swap");
	strcpy(config->nombreSwap, nombre_temp);
	config->cantidadPaginas = config_get_int_value(archivoConfig, "Cantidad_Paginas");
	config->tamanioPagina = config_get_int_value(archivoConfig, "Tamaño_Pagina");
	config->retardoCompactacion = config_get_int_value(archivoConfig, "Retardo_Compactacion");
}


void escucharUMC(){

	int sockServidor;

	sockServidor = nuevoSocket();
	asociarSocket(sockServidor, config->puerto);
	escucharSocket(sockServidor, 1);

	sockUMC = aceptarConexionSocket(sockServidor);
	printf("UMC conectada. Esperando mensajes\n");
	handshake_servidor(sockUMC, "Swap\n");

	char package[PACKAGESIZE];
	int status = 1;		// Estructura que manjea el status de los recieve.

	while (status > 0){
		status = recibirPorSocket(sockUMC, package, CHAR*2);
		package[1] = '\0';
		printf("UMC: %s\n", package);
	}

	close(sockUMC);
	close(sockServidor);
}


void liberarEstructuraConfig() {
	free(config->nombreSwap);
	free(config);
}

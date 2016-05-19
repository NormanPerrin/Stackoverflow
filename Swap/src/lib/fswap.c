#include "fswap.h"

// Globales
t_configuracion *config; // guarda valores config
int sockUMC; // socket cliente UMC

// Funciones
void setearValores_config(t_config * archivoConfig){
	config = (t_configuracion*)reservarMemoria(sizeof(t_configuracion));
	config->nombreSwap = (char*)reservarMemoria(CHAR*30);
	config->puerto = config_get_int_value(archivoConfig, "PUERTO_ESCUCHA");
	config->nombreSwap = strdup(config_get_string_value (archivoConfig, "NOMBRE_SWAP"));
	config->cantidadPaginas = config_get_int_value(archivoConfig, "CANTIDAD_PAGINAS");
	config->tamanioPagina = config_get_int_value(archivoConfig, "TAMANIO_PAGINA");
	config->retardoCompactacion = config_get_int_value(archivoConfig, "RETARDO_COMPACTACION");
}


void escucharUMC(){

	int sockServidor;

	sockServidor = nuevoSocket();
	asociarSocket(sockServidor, config->puerto);
	escucharSocket(sockServidor, 1);

	int ret_handshake = 0;
	while(ret_handshake == 0) { // Mientras que el cliente adecuado no se conecte

		sockUMC = aceptarConexionSocket(sockServidor);

		if ( validar_conexion(sockUMC, 0) == FALSE ) {
			continue;
		} else {
			ret_handshake = handshake_servidor(sockUMC, "S");
		}

	}

	int status = 1;		// Estructura que manjea el status de los recieve.
	uint8_t *head = (uint8_t*)reservarMemoria(1); // 0 .. 255

	while (status > 0){
		status = recibirPorSocket(sockUMC, head, 1);
		validar_recive(status, 1); // es terminante ya que si hay un error en el recive o desconexión debe terminar
		printf("%d\n", *head);
	}

	free(head);
	close(sockUMC);
	close(sockServidor);
}


void liberarEstructura() {
	free(config->nombreSwap);
	free(config);
}

void liberarRecusos() {
	// liberar otros recursos
	liberarEstructura();
}

int validar_cliente(char *id) {
	if(!strcmp(id, "U")) {
		printf("Cliente aceptado\n");
		return TRUE;
	} else {
		printf("Cliente rechazado\n");
		return FALSE;
	}
}
int validar_servidor(char *id){return 0;}

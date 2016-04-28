#include "fswap.h"

// Globales
t_configuracion *config;


// Funciones
void setearValores_config(t_config * archivoConfig){
	config = (t_configuracion*)reservarMemoria(sizeof(t_configuracion));
	puertoEscuchaUMC = config_get_int_value(archivoConfig, "PUERTO");
	config->nombreSwap = config_get_int_value (archivoConfig, "Nombre_Swap");
	config->cantidadPaginas = config_get_int_value(archivoConfig, "Cantidad_Paginas");
	config->tamanioPagina = config_get_int_value(archivoConfig, "Tamaniobo_Pagina");
	config->retardoCompactacion = config_get_int_value(archivoConfig, "Retardo_Compactacion");
}


void escucharUMC(){

	int fd_escuchaUMC, fd_nuevaUMC;

	fd_escuchaUMC = nuevoSocket();
	asociarSocket(fd_escuchaUMC, puertoEscuchaUMC);
	escucharSocket(fd_escuchaUMC, CONEXIONES_PERMITIDAS);
	fd_nuevaUMC = aceptarConexionSocket(fd_escuchaUMC);

	printf("UMC conectada. Esperando mensajes\n");
	/*escucharSocket(fd_escuchaUMC, CONEXIONES_PERMITIDAS);*/// Ponemos a esuchar de nuevo al socket escucha

	char package[PACKAGESIZE];
	int status = 1;		// Estructura que manjea el status de los recieve.
	// Vamos a ESPERAR que nos manden los paquetes, y los imprimos por pantalla
	while (status != 0){
		status = recibirPorSocket(fd_nuevaUMC, (void*) package, PACKAGESIZE);
		if (status != 0) printf("%s", package);
	}
	close(fd_nuevaUMC);
	close(fd_escuchaUMC);

}

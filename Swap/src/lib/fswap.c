#include "fswap.h"

t_configuracion *config;
// Funciones

void abrirArchivoConfig(char *ruta){

	 t_config *configuracion = config_create(ruta);
	 t_configuracion *ret = (t_configuracion*)reservarMemoria(sizeof(t_configuracion));


	   puertoEscuchaUMC = config_get_int_value(configuracion, "PUERTO");
	   ret->nombreSWap = config_get_int_value (configuracion, "Nombre_Swap");
	   ret->cantidadPaginas = config_get_int_value(configuracion, "Cantidad_Paginas");
	   ret->tamañioPagina = config_get_int_value(configuracion, "Tamaño_Pagina");
	   ret->retardoCompactacion = config_get_int_value(configuracion, "Retardo_Compactacion");

	 	config = ret;
}

void escucharAUMC(){
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

	} // Soy servidor, espero mensajes de la UMC

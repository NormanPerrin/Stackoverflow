#include "fswap.h"

// Funciones

void setearValores_config(t_config * archivoConfig){

// Desarrollar seteo

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

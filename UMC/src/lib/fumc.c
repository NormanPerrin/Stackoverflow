#include "fumc.h"

// Funciones

void setearValores_config(t_config * archivoConfig){

// Desarrollar seteo

}

void escucharANucleo(){
	int fd_escuchaNucleo, fd_nuevoNucleo;

				fd_escuchaNucleo = nuevoSocket();
				asociarSocket(fd_escuchaNucleo, puerto);
				escucharSocket(fd_escuchaNucleo, CONEXIONES_PERMITIDAS);
				fd_nuevoNucleo= aceptarConexionSocket(fd_escuchaNucleo);

				printf("Núcleo conectado. Esperando mensajes\n");
				esperarPaqueteDelCliente(fd_escuchaNucleo, fd_nuevoNucleo);

}

void escucharACPU(){
	int fd_escuchaCPU, fd_nuevoCPU;

				fd_escuchaCPU = nuevoSocket();
				asociarSocket(fd_escuchaCPU, puerto);
				escucharSocket(fd_escuchaCPU, CONEXIONES_PERMITIDAS);
				fd_nuevoCPU= aceptarConexionSocket(fd_escuchaCPU);

				printf("Núcleo conectado. Esperando mensajes\n");
				esperarPaqueteDelCliente(fd_escuchaCPU, fd_nuevoCPU);

}

void esperarPaqueteDelCliente(int fd_escucha, int fd_nuevoCliente){

	/*escucharSocket(fd_nuevoCliente, CONEXIONES_PERMITIDAS);*/// Ponemos a esuchar de nuevo al socket escucha

			char package[PACKAGESIZE];
				int status = 1;		// Estructura que manjea el status de los recieve.
				// Vamos a ESPERAR que nos manden los paquetes, y los imprimos por pantalla
				while (status != 0){
					status = recibirPorSocket(fd_nuevoCliente, (void*) package, PACKAGESIZE);
					if (status != 0) printf("%s", package);

				}
				close(fd_nuevoCliente);
					close(fd_escucha);

} // Soy servidor, espero mensajes de algún Cliente (CPU o Núcleo)

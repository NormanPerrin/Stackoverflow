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

} // Soy servidor, espero mensajes del Núcleo

void escucharACPU(){
	int fd_escuchaCPU, fd_nuevoCPU;

				fd_escuchaCPU = nuevoSocket();
				asociarSocket(fd_escuchaCPU, puerto);
				escucharSocket(fd_escuchaCPU, CONEXIONES_PERMITIDAS);
				fd_nuevoCPU= aceptarConexionSocket(fd_escuchaCPU);

				printf("CPU conectado. Esperando mensajes\n");
				esperarPaqueteDelCliente(fd_escuchaCPU, fd_nuevoCPU);

} // Soy servidor, espero mensajes de algún CPU

void conectarConSwap(){
	int fd_serverSwap;

	fd_serverSwap = nuevoSocket();
			asociarSocket(fd_serverSwap, puertoSwap);
			conectarSocket(fd_serverSwap, ipSwap, puertoSwap);
			// Creo un paquete (string) de size PACKAGESIZE, que le enviaré al Swap
			int enviar = 1;
				char message[PACKAGESIZE];

				printf("Conectado al Swap. Ya se puede enviar mensajes. Escriba 'exit' para salir\n");

				while(enviar){
					fgets(message, PACKAGESIZE, stdin);	// Lee una línea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE
					if (!strcmp(message,"exit\n")) enviar = 0; // Chequeo que no se quiera salir
					if (enviar) enviarPorSocket(fd_serverSwap, message, strlen(message) + 1); // Sólo envío si no quiere salir
				}
				close(fd_serverSwap);
		} // Soy cliente del Swap, es  decir, soy el que inicia la conexión con él

void esperarPaqueteDelCliente(int fd_escucha, int fd_nuevoCliente){

	/*escucharSocket(fd_escucha, CONEXIONES_PERMITIDAS);*/// Ponemos a esuchar de nuevo al socket escucha

			char package[PACKAGESIZE];
				int status = 1;		// Estructura que manjea el status de los recieve.
				// Vamos a ESPERAR que nos manden los paquetes, y los imprimos por pantalla
				while (status != 0){
					status = recibirPorSocket(fd_nuevoCliente, (void*) package, PACKAGESIZE);
					if (status != 0) printf("%s", package);

				}
				close(fd_nuevoCliente);
					close(fd_escucha);
}

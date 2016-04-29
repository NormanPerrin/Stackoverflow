#include "fconsola.h"

// Funciones

void setearValores_config(t_config * archivoConfig){
	puertoNucleo = config_get_int_value(archivoConfig, "PUERTO_NUCLEO");
}

void conectarConNucleo(){
int fd_serverConsola;

	fd_serverConsola = nuevoSocket();
	asociarSocket(fd_serverConsola, puertoNucleo);
	conectarSocket(fd_serverConsola, ipNucleo, puertoNucleo);
	// Creo un paquete (string) de size PACKAGESIZE, que le enviaré al Núcleo
	int enviar = 1;
		char message[PACKAGESIZE];

		printf("Conectado al Núcleo. Ya se puede enviar mensajes. Escriba 'exit' para salir\n");

		while(enviar){
			fgets(message, PACKAGESIZE, stdin);	// Lee una línea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE
			if (!strcmp(message,"exit\n")) enviar = 0; // Chequeo que no se quiera salir
			if (enviar) enviarPorSocket(fd_serverConsola, message, strlen(message) + 1); // Sólo envío si no quiere salir
		}
		close(fd_serverConsola);
} // Soy cliente del Núcleo, es  decir, soy el que inicia la conexión con él

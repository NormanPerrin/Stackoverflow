#include "fcpu.h"

// Funciones

void setearValores_config(t_config * archivoConfig){
	ipNucleo = config_get_int_value(archivoConfig, "IP_NUCLEO");
	ipUMC = config_get_int_value(archivoConfig, "IP_UMC");
}

void conectarConNucleo(){
int fd_serverCPU;

	fd_serverCPU = nuevoSocket();
	asociarSocket(fd_serverCPU, puertoNucleo);
	conectarSocket(fd_serverCPU, ipNucleo, puertoNucleo);
	// Creo un paquete (string) de size PACKAGESIZE, que le enviaré al Núcleo
	int enviar = 1;
		char message[PACKAGESIZE];

		printf("Conectado al Núcleo. Ya se puede enviar mensajes. Escriba 'exit' para salir\n");

		while(enviar){
			fgets(message, PACKAGESIZE, stdin);	// Lee una línea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE
			if (!strcmp(message,"exit\n")) enviar = 0; // Chequeo que no se quiera salir
			if (enviar) enviarPorSocket(fd_serverCPU, message, strlen(message) + 1); // Sólo envío si no quiere salir
		}
		close(fd_serverCPU);
} // Soy cliente del Núcleo, es  decir, soy el que inicia la conexión con él

void conectarConUMC(){
int fd_serverCPU;

	fd_serverCPU = nuevoSocket();
	asociarSocket(fd_serverCPU, puertoUMC);
	conectarSocket(fd_serverCPU, ipUMC, puertoUMC);
	// Creo un paquete (string) de size PACKAGESIZE, que le enviaré a la UMC
	int enviar = 1;
		char message[PACKAGESIZE];

		printf("Conectado a la UMC. Ya se puede enviar mensajes. Escriba 'exit' para salir\n");

		while(enviar){
			fgets(message, PACKAGESIZE, stdin);	// Lee una línea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE
			if (!strcmp(message,"exit\n")) enviar = 0; // Chequeo que no se quiera salir
			if (enviar) enviarPorSocket(fd_serverCPU, message, strlen(message) + 1); // Sólo envío si no quiere salir
		}
		close(fd_serverCPU);
} // Soy cliente de la UMC, es  decir, soy el que inicia la conexión con ella

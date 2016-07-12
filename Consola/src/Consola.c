#include <stdlib.h>
#include "lib/fconsola.h"

int main(int argc, char **argv) {

	system("clear");
	setvbuf(stdout, NULL, _IONBF, 0);

	validar_argumentos(argc);

	crearLoggerConsola();

	leerArchivoDeConfiguracion(RUTA_CONFIG_CONSOLA);

	leerScript(argv[1]);

	conectarCon_Nucleo(); // Conexión con Núcleo

	aplicar_protocolo_enviar(fd_nucleo, ENVIAR_SCRIPT, programa);

	// Espera activa de mensajes:
	int head;
	void * mensaje = aplicar_protocolo_recibir(fd_nucleo, &head);

		if (head == RECHAZAR_PROGRAMA) {
			puts("La UMC no pudo alocar los segmentos pedidos. El programa ha sido rechazado.\n");
			cerrarSocket(fd_nucleo);
			liberarRecursos(); // Libera memoria asignada

			return EXIT_FAILURE;
			}
		free(mensaje); mensaje = NULL;

		puts("Escuchando nuevos mensajes del Núcleo.\n");
		while(TRUE) {

			void * mensaje = aplicar_protocolo_recibir(fd_nucleo, &head);
			switch(head){

			case IMPRIMIR_TEXTO:{
				// Imprime lo que recibe, ya sea texto a variable (convertida a texto):
				puts((char*)mensaje);
				free(mensaje); mensaje = NULL;
				break;
				}
			case FINALIZAR_PROGRAMA:{
				puts("El programa ha finalizado con éxito. Cerrando el programa.\n");
				free(mensaje); mensaje = NULL;
				cerrarSocket(fd_nucleo);
				liberarRecursos(); // Libera memoria asignada

				return EXIT_SUCCESS;
				break;
				}
			} // fin del switch-case
		} // fin del while
	return EXIT_SUCCESS;
}

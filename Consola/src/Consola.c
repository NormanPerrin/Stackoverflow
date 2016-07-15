#include <stdlib.h>
#include "lib/fconsola.h"

int main(int argc, char **argv){

	system("clear");
	setvbuf(stdout, NULL, _IONBF, 0);

	validar_argumentos(argc);

	crearLoggerConsola();
	leerArchivoDeConfiguracion(RUTA_CONFIG_CONSOLA);

	leerScript(argv[1]);

	conectarCon_Nucleo(); // Conexión con Núcleo

	printf("Script enviado a Núcleo. Esperando respuesta...\n");
	aplicar_protocolo_enviar(fd_nucleo, ENVIAR_SCRIPT, programa);

	int head;
	int* respuesta = malloc(INT);
	void * mensaje = NULL;
	mensaje = aplicar_protocolo_recibir(fd_nucleo, &head); // Recibo respuesta de incio programa

	if (head == PROGRAMA_NEW){
		respuesta = (int*) mensaje;

		switch(*respuesta){

	case RECHAZADO:{ // programa rechazado
				puts("La UMC no pudo alocar los segmentos pedidos. El programa ha sido rechazado.\n");
				exitConsola();
				return EXIT_FAILURE;
				break;
			} // fin case rechazado

	case ERROR_CONEXION:{
				printf("Error al iniciar programa. Script no recibido.\n");
				exitConsola();
				return EXIT_FAILURE;
				break;
			} // fin case error conexión

	case ACEPTADO:{ // programa aceptado
				puts("Escuchando nuevos mensajes de Núcleo...\n");

		while(TRUE){ // Espera activa de mensajes

				int protocolo;
				void * entrada = NULL;
				entrada = aplicar_protocolo_recibir(fd_nucleo, &protocolo);

			switch(protocolo){

			case IMPRIMIR_TEXTO:{
						// Imprime lo que recibe, ya sea texto a variable (convertida a texto):
						printf("Imprimir: '%s.", (char*)entrada);
						break;
					}
			case FINALIZAR_PROGRAMA:{
						puts("El programa ha finalizado con éxito. Cerrando proceso Consola.\n");
						exitConsola();
						return EXIT_SUCCESS;
						break;
					}
			default:
				printf("Se ha recibido un mensaje inválido de Núcleo.");
				break;
				} // fin switch-case nuevos mensajes
			} // fin while espera mensajes
					break;
	}
		} // fin switch respuesta inicio
		free(respuesta); respuesta = NULL;
	} // fin if head válido

	printf("Error: No se recibió respuesta de inicio de programa de Núcleo.\n");
	exitConsola();
	return EXIT_FAILURE;

} // fin main

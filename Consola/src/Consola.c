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

	aplicar_protocolo_enviar(fd_nucleo, ENVIAR_SCRIPT, programa);
	log_info(logger, "Script enviado a Núcleo. Esperando respuesta.");

	int head;
	void * mensaje = NULL;
	mensaje = aplicar_protocolo_recibir(fd_nucleo, &head); // Recibo respuesta de incio programa

	if (head == PROGRAMA_NEW){

		switch(*((int*) mensaje)){

	case RECHAZADO:{ // programa rechazado
				log_info(logger, "UMC no pudo alocar los segmentos pedidos. Programa rechazado.");
				free(mensaje); mensaje = NULL;
				exitConsola();
				return EXIT_FAILURE;
				break;
			} // fin case rechazado

	case ERROR_CONEXION:{
				log_error(logger, "Error al iniciar programa. Script no enviado.");
				free(mensaje); mensaje = NULL;
				exitConsola();
				return EXIT_FAILURE;
				break;
			} // fin case error conexión

	case ACEPTADO:{ // programa aceptado
				log_info(logger, "Programa aceptado. Escuchando nuevos mensajes de Núcleo...");

		while(TRUE){ // Espera activa de mensajes

				int protocolo;
				void * entrada = NULL;
				entrada = aplicar_protocolo_recibir(fd_nucleo, &protocolo);
				if(entrada == NULL) break;

			switch(protocolo){

			case IMPRIMIR_TEXTO:{
						// Imprime lo que recibe, ya sea texto a variable (convertida a texto):
						printf(">> IMPRIMIR: '%s'.\n", (char*) entrada);
						free(entrada); entrada = NULL;
						break;
					}
			case FINALIZAR_PROGRAMA:{
						int respuesta = *((int*)entrada);
						if(respuesta == PERMITIDO) {
							log_info(logger, "El programa ha finalizado con éxito.");
						} else {
							log_error(logger, "El programa ha sido abortado.");
						}
						free(entrada); entrada = NULL;
						free(mensaje); mensaje = NULL;
						exitConsola();
						return EXIT_SUCCESS;
						break;
					}
				} // fin switch-case nuevos mensajes
			} // fin while espera mensajes
		break;
		} // fin case aceptado
	} // fin switch respuesta inicio
	} // fin if head válido
	free(mensaje); mensaje = NULL;
	exitConsola();
	return EXIT_FAILURE;

} // fin main

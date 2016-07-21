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

	string* script = malloc(tamanioPrograma + 4);
	script->tamanio = tamanioPrograma;
	script->cadena = programa;
	aplicar_protocolo_enviar(fd_nucleo, ENVIAR_SCRIPT, script);
	printf("Script enviado a Núcleo. Esperando respuesta...\n");
	free(script->cadena);
	free(script);

	int head;
	int* respuesta = malloc(INT);
	void * mensaje = NULL;
	mensaje = aplicar_protocolo_recibir(fd_nucleo, &head); // Recibo respuesta de incio programa

	if (head == PROGRAMA_NEW){
		respuesta = (int*) mensaje;

		switch(*respuesta){

	case RECHAZADO:{ // programa rechazado
				log_info(logger, "La UMC no pudo alocar los segmentos pedidos. Programa rechazado.");
				exitConsola();
				return EXIT_FAILURE;
				break;
			} // fin case rechazado

	case ERROR_CONEXION:{
				log_error(logger, "Error al iniciar programa. Script no enviado.");
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
						printf("IMPRIMIR: %s.", (char*)entrada);
						break;
					}
			case FINALIZAR_PROGRAMA:{
						int respuesta = *((int*)entrada);
						if(respuesta == PERMITIDO) {
							puts("El programa ha finalizado con éxito.\n");
						} else {
							puts("El programa no ha finalizado con éxito.\n");
						}
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
		} // fin case aceptado
	} // fin switch respuesta inicio
		free(respuesta); respuesta = NULL;
	} // fin if head válido

	log_error(logger, "No se recibió respuesta de inicio de programa de Núcleo.");
	exitConsola();
	return EXIT_FAILURE;

} // fin main

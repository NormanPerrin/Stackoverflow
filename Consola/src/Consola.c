#include <stdlib.h>
#include "lib/fconsola.h"

int main(int argc, char **argv){

	system("clear");
	setvbuf(stdout, NULL, _IONBF, 0);

//	validar_argumentos(argc);

	crearLoggerConsola();
	leerArchivoDeConfiguracion(RUTA_CONFIG_CONSOLA);

	//leerScript(argv[1]);
	leerScript("/home/utnso/tp-2016-1c-Cazadores-de-cucos/Consola/Debug/facil.ansisop");

	conectarCon_Nucleo(); // Conexión con Núcleo

	printf("Script enviado a Núcleo. Esperando respuesta...\n");
	aplicar_protocolo_enviar(fd_nucleo, ENVIAR_SCRIPT, programa);

	int head;
	int* respuesta = malloc(INT);
	void * mensaje = aplicar_protocolo_recibir(fd_nucleo, &head); // Recibo respuesta de incio programa

	if (head == RECHAZAR_PROGRAMA){
		respuesta = (int*) mensaje;

		switch(*respuesta){

	case TRUE:{ // programa aceptado

			free(mensaje); mensaje = NULL;
			puts("Escuchando nuevos mensajes de Núcleo.\n");

			while(TRUE){ // Espera activa de mensajes

			int protocolo;
			void * mensaje = aplicar_protocolo_recibir(fd_nucleo, &protocolo);

			switch(protocolo){

		case IMPRIMIR_TEXTO:{
					// Imprime lo que recibe, ya sea texto a variable (convertida a texto):
					puts((char*)mensaje);
					free(mensaje); mensaje = NULL;
					break;
				}
		case FINALIZAR_PROGRAMA:{
					puts("El programa ha finalizado con éxito. Cerrando proceso Consola.\n");
					free(mensaje); mensaje = NULL;
					cerrarSocket(fd_nucleo);
					liberarRecursos(); // Libera memoria asignada
					return EXIT_SUCCESS;
					break;
				}
			} // fin switch-case nuevos mensajes
		} // fin while espera mensajes
				break;
			} // fin case true
	case FALSE:{ // programa rechazado
				puts("La UMC no pudo alocar los segmentos pedidos. El programa ha sido rechazado.\n");
				free(mensaje); mensaje = NULL;
				cerrarSocket(fd_nucleo);
				liberarRecursos(); // Libera memoria asignada
				printf("Cerrando proceso Consola.\n");
				return EXIT_FAILURE;
				break;
			} // fin case false
	case ERROR:{
				printf("Error al iniciar programa. Ingresar nuevamente el script...\n");
				free(mensaje); mensaje = NULL;
				cerrarSocket(fd_nucleo);
				liberarRecursos(); // Libera memoria asignada
				printf("Cerrando proceso Consola.\n");
				return EXIT_FAILURE;
				break;
			} // fin case error
		} // fin switch respuesta inicio
	} // fin if head válido

	printf("Error: No se recibió respuesta de inicio de programa de Núcleo.\n");
	free(mensaje); mensaje = NULL;
	cerrarSocket(fd_nucleo);
	liberarRecursos(); // Libera memoria asignada
	printf("Cerrando proceso Consola.\n");
	return EXIT_FAILURE;

} // fin main

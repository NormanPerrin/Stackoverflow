#ifndef UTILIDADES_GENERAL_H_
#define UTILIDADES_GENERAL_H_

	#include <commons/config.h>
	#include <errno.h>	// Incluye perror
	#include "sockets.h"
	#include <unistd.h>
	#include <string.h>

	#define FALSE 0
	#define TRUE 1
	#define ERROR -1
	#define INT (sizeof(int))
	#define CHAR (sizeof(char))
	#define manejarError(mensajeError) {perror(mensajeError);} //--abort(); Función para el tratamiento de errores
	#define HANDSHAKE_SIZE CHAR*2 // Tamaño de consola\0

	void* reservarMemoria(int size); // Hace lo mismo que el malloc pero se fija si hay error. Necesita casteo como malloc
	void leerArchivoDeConfiguracion(char * ruta);
	int comprobarQueExistaArchivo(char * ruta);
	void setearValores_config(t_config * archivoConfig); // Hay que redefinirlo en cada proceso
	int handshake_servidor(int sockCliente, char *mensaje); // Envía mensaje handshake al socketCliente y luego se queda esperando mensaje de confirmación
	void handshake_cliente(int sockClienteDe, char *mensaje); // Se queda en espera de mensaje de servidor y luego envía un mensaje
	int validar_cliente(char *id); // Valida que la conexión del cliente sea correcta. Ej: para Nucleo, Consola
	int validar_servidor(char *id); // Valida que la conexión al servidor sea correcta. Ej: para UMC, CPU
	int validar_conexion(int socket, int modo); // Decide si termina o no el programa ante un error, según el modo. Modo 1 es terminante. Con modo no terminante retorna: 0 en ERROR. 1 en éxito
	int validar_recive(int bytes, int modo); // Decide si ante una desconexión o error en mensaje termina el programa o sigue, según el modo. Modo 1 es terminante. Retorno: 0 en desconexión o error. 1 en éxito

#endif

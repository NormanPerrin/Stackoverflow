#ifndef UTILIDADES_GENERAL_H_
#define UTILIDADES_GENERAL_H_

	#include <commons/config.h>
	#include <stdint.h>
	#include <errno.h>	// Incluye perror
	#include "sockets.h"
	#include <unistd.h>
	#include <string.h>
	#include <time.h>
	#include <commons/log.h>
	#include "tiposDeDatos.h"

	#define ERROR -1
	#define INT (sizeof(int))
	#define CHAR (sizeof(char))
	#define DIRECCION (sizeof(direccion))
	#define VARIABLE (sizeof(variable))
	#define HANDSHAKE_SIZE CHAR*2 // Tamaño de consola\0
	#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

	void* reservarMemoria(int size); // Hace lo mismo que el malloc pero se fija si hay error. Necesita casteo como malloc
	void leerArchivoDeConfiguracion(char * ruta);
	int comprobarQueExistaArchivo(char * ruta);
	void setearValores_config(t_config * archivoConfig); // Hay que redefinirlo en cada proceso
	int handshake_servidor(int sockCliente, char *mensaje); // Envía mensaje handshake al socketCliente y luego se queda esperando mensaje de confirmación
	void handshake_cliente(int sockClienteDe, char *mensaje); // Se queda en espera de mensaje de servidor y luego envía un mensaje
	int validar_cliente(char *id); // Valida que la conexión del cliente sea correcta
	int validar_servidor(char *id); // Valida que la conexión al servidor sea correcta
	int validar_conexion(int socket, int modo); // Decide si termina o no el programa ante un error, según el modo. Modo 1 es terminante. Con modo no terminante retorna: 0 en ERROR. 1 en éxito
	int validar_recive(int bytes, int modo); // Decide si ante una desconexión o error en mensaje termina el programa o sigue, según el modo. Modo 1 es terminante. Retorno: 0 en desconexión o error. 1 en éxito
	void dormir(float miliseconds); // Duerme el hilo del que se está ejecutando milisegundos (más preciso y consistente que sleep y usleep)
	registroStack* reg_stack_create();

#endif

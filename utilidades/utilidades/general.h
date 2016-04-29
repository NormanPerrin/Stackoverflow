#ifndef UTILIDADES_GENERAL_H_
#define UTILIDADES_GENERAL_H_

#include <commons/config.h>
#include <errno.h>	// Incluye perror
#include "sockets.h"

#define FALSE 0
#define TRUE 1
#define ERROR -1
#define INT (sizeof(int))
#define CHAR (sizeof(char))
#define manejarError(mensajeError) {perror(mensajeError); abort();} // Función para el tratamiento de errores

	/**
	* @NAME: reservarMemoria
	* @DESC: Crea dinámicamente y devuelve un puntero a void
	* @PARAMS:
	*		size - Tamaño en bytes del puntero
	*/
	void* reservarMemoria(int size);

	// Lectura Archivo de Configuración
	void leerArchivoDeConfiguracion(char * ruta);
	int comprobarQueExistaArchivo(char * ruta);
	void setearValores_config(t_config * archivoConfig); // Hay que redefinirlo en cada proceso (Ejemplo en Núcleo)
	void handshake_servidor(int sockCliente, char *mensaje); // Envía mensaje handshake al socketCliente y luego se queda esperando mensaje de confirmación
	void handshake_cliente(int sockClienteDe, char *mensaje);

#endif

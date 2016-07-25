#ifndef UTILIDADES_SOCKETS_H_
#define UTILIDADES_SOCKETS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>     // memset
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>      // perror
#include <arpa/inet.h>  // INADDR_ANY
#include <unistd.h>     // close, usleep
#include <netdb.h> 		// gethostbyname
#include <netinet/in.h>
#include <fcntl.h> // fcntl

#define FALSE 0
#define TRUE 1
#define ERROR -1 // Las system-calls de sockets retornan -1 en caso de error
#define manejarError(msjError) {perror(msjError);} // Tratamiento de errores

// *******************************************************************
// *           FUNCIONES GENERALES PARA TRABAJAR CON SOCKETS		 *
// *******************************************************************

// Funciones para el Servidor
int  nuevoSocket();
struct sockaddr_in asociarSocket(int fd_socket, int puerto);
void escucharSocket(int fd_socket, int conexionesEntrantesPermitidas);
int  aceptarConexionSocket(int fd_socket);

// Función para el Cliente
int conectarSocket(int fd_socket, const char * ipDestino, int puerto);

// Funciones para el Envío y la Recepción de datos
int enviarPorSocket(int fdServidor, const void * mensaje, int tamanioBytes); // Retorna el número de bytes enviados, o bien, -1 si falla y 0 si se desconecta
int recibirPorSocket(int fdCliente, void * buffer, int tamanioBytes) ; // Retorna el número de bytes recibidos, o bien, -1 si falla y 0 si se desconecta

// Función para cerrar un File Descriptor
void cerrarSocket(int fd_socket);

// Función para comprobar varios Sockets al mismo tiempo (Select)
void seleccionarSocket(int mayorValorDeFD,
		fd_set * fdListosParaLectura, fd_set * fdListosParaEscritura, fd_set * fdListosParaEjecucion,
			int* segundos, int* milisegundos);

#endif /* UTILIDADES_SOCKETS_H_ */

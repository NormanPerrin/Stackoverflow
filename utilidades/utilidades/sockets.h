#ifndef UTILIDADES_SOCKETS_H_
#define UTILIDADES_SOCKETS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>     // Incluye memset
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>      // Incluye perror
#include <arpa/inet.h>  // Incluye INADDR_ANY
#include <unistd.h>     // Incluye close, usleep
#include <netdb.h> 		// Incluye gethostbyname
#include <netinet/in.h>
#include <fcntl.h> // Incluye fcntl
#include <commons/error.h>

#define FALSE 0
#define TRUE 1
#define ERROR -1 // Las llamadas al sistema utilizadas retornan -1 en caso de error
#define manejarError(mensajeError) {perror(mensajeError);} // Función para el tratamiento de errores, muestra mensaje a través de perror


// *******************************************************************
// *           FUNCIONES GENERALES PARA TRABAJAR CON SOCKETS		 *
// *******************************************************************

// Funciones para el Servidor
int  nuevoSocket();
struct sockaddr_in asociarSocket(int fd_socket, int puerto);
void escucharSocket(int fd_socket, int conexionesEntrantesPermitidas); // El segundo int corresponde al backlog (conexiones permitidas en la cola de entrada)
int  aceptarConexionSocket(int fd_socket);

// Función para el Cliente
int conectarSocket(int fd_socket, char * ipDestino, int puerto);

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

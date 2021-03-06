#include "sockets.h" // Incluimos nuestro propio header de Sockets

// *******************************
// *           Servidor			 *
// *******************************

// Creación del socket
int nuevoSocket(){
	int fileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
	if (fileDescriptor == ERROR) printf("[SOCKETS] No se pudo crear el file descriptor.\n");

	return fileDescriptor;
}

// Asociación del socket con algún puerto local
struct sockaddr_in asociarSocket(int fd_socket, int puerto){
	struct sockaddr_in miDireccionSocket;

	miDireccionSocket.sin_family = AF_INET;
	miDireccionSocket.sin_port = htons(puerto);
	miDireccionSocket.sin_addr.s_addr = 0; // Con htons(INADDR_ANY) (o bien, 0) usa la dirección IP de la máquina actual
	memset(&(miDireccionSocket.sin_zero), '\0', 8); // Rellena con ceros el resto de la estructura

// Si el puerto ya está siendo utilizado, lanzamos un error
	int enUso = 1;
	int puertoYaAsociado = setsockopt(fd_socket, SOL_SOCKET, SO_REUSEADDR, (char*) &enUso, sizeof(enUso));
	if (puertoYaAsociado == ERROR) printf("[SOCKETS] El puerto a asociar ya está siendo utilizado.\n");

// Ya comprobado el error de puerto, llamamos a bind
	int retornoBind = bind(fd_socket, (struct sockaddr *) &miDireccionSocket, sizeof(struct sockaddr));
	if (retornoBind == ERROR) printf("[SOCKETS] No se pudo asociar el socket al puerto.\n");

	return miDireccionSocket;
}

// Ponemos al socket a escuchar conexiones entrantes
void escucharSocket(int fd_socket, int conexionesEntrantesPermitidas){
	int retornoListen = listen(fd_socket, conexionesEntrantesPermitidas); // SOMAXCONN: máximo tamaño de la cola
	if (retornoListen == ERROR) printf("[SOCKETS] No se pudo poner al socket a escuchar conexiones.\n");
}

// Obtención de una conexión entrante pendiente
int aceptarConexionSocket(int fd_socket) {
	struct sockaddr_storage unCliente; // sino: struct sockaddr_in unCliente;
	unsigned int addres_size = sizeof(unCliente);

	int fdCliente = accept(fd_socket, (struct sockaddr*) &unCliente, &addres_size);
	if(fdCliente == ERROR) printf("[SOCKETS] No se pudo aceptar la conexión entrante.\n");

	return fdCliente;
}

// *******************************
// *           Cliente			 *
// *******************************

// Conexión del socket con una máquina remota (Servidor)
int conectarSocket(int fd_socket, const char * ipDestino, int puerto){
	struct sockaddr_in direccionServidor;

	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_port = htons(puerto);
	direccionServidor.sin_addr.s_addr = inet_addr(ipDestino); // sino: inet_aton(ipDestino, &(direccionServidor.sin_addr));
	memset(&(direccionServidor.sin_zero), '\0', 8);

	int retornoConnect = connect(fd_socket, (struct sockaddr *) &direccionServidor, sizeof(struct sockaddr));
	if (retornoConnect == ERROR) {
		printf("[SOCKETS] No se pudo realizar la conexión entre el socket y el servidor.\n");
		return ERROR;
	} else {
		return 0;
	}
}
// **********************************
// *    Enviar y Recibir Datos	    *
// **********************************

// Enviar algo a través de sockets
int enviarPorSocket(int fdCliente, const void * mensaje, int tamanioBytes) {
	int bytes_enviados;
	int totalBytes = 0;

	while (totalBytes < tamanioBytes) {
		bytes_enviados = send(fdCliente, mensaje + totalBytes, tamanioBytes, MSG_NOSIGNAL); // El 0 significa que no le paso ningún Flag
/* send: devuelve el múmero de bytes que se enviaron en realidad, pero como estos podrían ser menos
 * de los que pedimos que se enviaran, realizamos la siguiente validación: */

		if (bytes_enviados == ERROR) {
			break;
		}
		totalBytes += bytes_enviados;
		tamanioBytes -= bytes_enviados;
	}
	if (bytes_enviados == ERROR) printf("[SOCKETS] No se pudo enviar correctamente los datos.\n");

	return bytes_enviados; // En caso de éxito, se retorna la cantidad de bytes realmente enviada
}

// Recibir algo a través de sockets
int recibirPorSocket(int fdServidor, void * buffer, int tamanioBytes) {
	int total = 0;
	int bytes_recibidos;

	while (total < tamanioBytes){

	bytes_recibidos = recv(fdServidor, buffer+total, tamanioBytes, MSG_WAITALL);
	// MSG_WAITALL: el recv queda completamente bloqueado hasta que el paquete sea recibido completamente

	if (bytes_recibidos == ERROR) { // Error al recibir mensaje
		perror("[SOCKETS] No se pudo recibir correctamente los datos.\n");
		break;
			}

	if (bytes_recibidos == 0) { // Conexión cerrada
		printf("[SOCKETS] La conexión fd #%d se ha cerrado.\n", fdServidor);
		break;
	}
	total += bytes_recibidos;
	tamanioBytes -= bytes_recibidos;
		}
	return bytes_recibidos; // En caso de éxito, se retorna la cantidad de bytes realmente recibida
}

// ***********************************
// *    Cerrar un File Descriptor	 *
// ***********************************

// Cierre de la conexión del file descriptor del socket
void cerrarSocket(int fd_socket) {
	int retornoClose = close(fd_socket);
	if (retornoClose == ERROR) printf("[SOCKETS] No se pudo cerrar el file descriptor.\n");
}

// ******************************************
// *    Selección entre Sockets Listos	    *
// ******************************************

/* select: permite comprobar varios sockets al mismo tiempo.
Indica cuáles están listos para leer o para escribir, y cuáles han generado excepciones.
Le pasamos nuestro conjunto de FD de sockets Listos y los parámetros de tiempo */
void seleccionarSocket(int mayorValorDeFD,
		fd_set *fdListosParaLectura, fd_set *fdListosParaEscritura, fd_set *fdListosParaEjecucion,
		int* segundos, int* milisegundos) {

	inicioSeleccion:

/* Si segundos o microsegundos valen NULL:
 el temporizador nunca expirará y se tendrá que esperar hasta que algún FD esté listo */
	if(segundos == NULL || milisegundos == NULL){

	int retornoSelect = select((mayorValorDeFD + 1), fdListosParaLectura, fdListosParaEscritura, fdListosParaEjecucion, NULL);

	if(retornoSelect == ERROR && errno != EINTR) printf("[SOCKETS] No se pudo seleccionar ningún fd del conjunto.\n");

	}else{
/* Si segundos y microsegundos valen cero:
select regresará inmediatamente después de interrogar a todos los FD incluidos en los conjuntos de Listos*/
		struct timeval periodoMaximoDeEspera;

		int seg = *segundos; // segundos
		int microSeg = (*milisegundos) * 1000; // microsegundos

		periodoMaximoDeEspera.tv_sec  = seg;
		periodoMaximoDeEspera.tv_usec = microSeg;

		int selectConLimiteDeTiempo = select((mayorValorDeFD + 1), fdListosParaLectura,
								  fdListosParaEscritura, fdListosParaEjecucion, &periodoMaximoDeEspera);
		if(selectConLimiteDeTiempo == ERROR && errno != EINTR)printf("[SOCKETS] El temporizador del select() ha expirado.\n");
	}

	if (errno == EINTR){
		errno = 0;

		goto inicioSeleccion;
	}
}

#include "sockets.h" // Incluimos nuestro propio header de Sockets

// *******************************
// *           Servidor			 *
// *******************************

// Creación del socket
int nuevoSocket() {
	int fileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
		if (fileDescriptor == ERROR) manejarError("Error: No se pudo crear el socket");
	return fileDescriptor;
}

// Asociación del socket con algún puerto local
struct sockaddr_in asociarSocket(int sockfd, int puerto) {
	struct sockaddr_in miDireccionSocket;

	miDireccionSocket.sin_family = AF_INET;
	miDireccionSocket.sin_port = htons(puerto);
	miDireccionSocket.sin_addr.s_addr = 0; // Al poner en cero este campo, le estamos indicando a la función que elija un IP por nosotros
	memset(&(miDireccionSocket.sin_zero), '\0', 8); // Rellena con ceros el resto de la estructura

// Si el puerto ya está siendo utilizado, lanzamos un error
	int enUso = 1;
	int puertoYaAsociado = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*) &enUso, sizeof(enUso));
		if (puertoYaAsociado == ERROR)	manejarError("Error: [Puerto] La dirección de socket ya está siendo utilizada");

// Ya comprobado el error de puerto, llamamos a bind
	int retornoBind = bind(sockfd, (struct sockaddr *) &miDireccionSocket,sizeof(miDireccionSocket));
		if ( retornoBind == ERROR) manejarError("Error: No se pudo asociar el socket a un puerto");

	return miDireccionSocket;
}

// Ponemos al socket a escuchar conexiones entrantes
void escucharSocket(int sockfd, int conexionesEntrantesPermitidas) {
	int retornoListen = listen(sockfd, conexionesEntrantesPermitidas); // SOMAXCONN: máximo tamaño de la cola
		if ( retornoListen == ERROR) manejarError("Error: No se pudo poner al socket a escuchar conexiones entrantes");
		}

// Obtención de una conexión entrante pendiente
int aceptarConexionSocket(int sockfd) { // TODO RESPONDIDO (VER): Cambié manejarError por error_ show (sin abort)
	struct sockaddr_storage unCliente;
	unsigned int addres_size = sizeof(unCliente);

	int fdCliente = accept(sockfd, (struct sockaddr*) &unCliente, &addres_size);
		if(fdCliente == ERROR) error_show("No se pudo obtener una conexión entrante pendiente");

	return fdCliente;
}

// *******************************
// *           Cliente			 *
// *******************************

// Conexión del socket con una máquina remota (Servidor)
int conectarSocket(int sockfd, const char * ipDestino, int puerto){
	struct sockaddr_in direccionServidor;

	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_port = htons(puerto);
	direccionServidor.sin_addr.s_addr = inet_addr(ipDestino);
	memset(&(direccionServidor.sin_zero), '\0', 8);

	int retornoConnect = connect(sockfd, (struct sockaddr *) &direccionServidor, sizeof(struct sockaddr));
		if ( retornoConnect == ERROR) manejarError("Error: No se pudo realizar la conexión entre el socket y el servidor");

	return 0;
}
// **********************************
// *    Enviar y Recibir Datos	    *
// **********************************

// Enviar sirve para la comunicación a través de sockets
int enviarPorSocket(int fdCliente, const void * mensaje, int tamanioBytes) {
	int bytes_enviados;
	int totalBytes = 0;

	while (totalBytes < tamanioBytes) {
		bytes_enviados = send(fdCliente, mensaje + totalBytes, tamanioBytes, 0); /* send: devuelve el múmero de bytes que se enviaron en realidad,
		pero estos podrían ser menos de los que pedimos que se enviaran, por eso realizamos la siguiente validación */
		// El último argumento, cero, significa que no le paso ningún Flag
		if (bytes_enviados == ERROR) {
			break;
		}
		totalBytes += bytes_enviados;
		tamanioBytes -= bytes_enviados;
	}
	if (bytes_enviados == ERROR) manejarError ("Error: No se pudo enviar correctamente los datos entre los sockets");

	return bytes_enviados; // En caso de éxito, se retorna la cantidad de bytes realmente enviada
}

// Recibir sirve para la comunicación a través de sockets
int recibirPorSocket(int fdCliente, void * buffer, int tamanioBytes) { // TODO RESPONDIDO (VER): ver si con algún flag de recv se puede reeplazar al while -> Mm, por lo visto no, leí sobre los flags de recv acá: http://man7.org/linux/man-pages/man2/recv.2.html, y no hay nada útil
	int bytes_recibidos;
	int totalBytes = 0;

	while (totalBytes < tamanioBytes) {
		bytes_recibidos = recv(fdCliente, buffer + totalBytes, tamanioBytes, 0);
		// El último argumento, cero, significa que no le paso ningún Flag
		// Aquí también se analiza el caso de error
		// Desconexión: si el valor de retorno es ERROR o cero, significa que la máquina remota ha cerrado la conexión con el cliente

		if ((bytes_recibidos == ERROR) || (bytes_recibidos == 0)) {
			perror("Error: El servidor ha cerrado la conexión y no se pudo recibir correctamente los datos.");
			break;
		}

		totalBytes += bytes_recibidos;
		tamanioBytes -= bytes_recibidos;
	}
	return bytes_recibidos; // En caso de éxito, se retorna la cantidad de bytes realmente recibida
}

// ***********************************
// *    Cerrar un File Descriptor	 *
// ***********************************

// Cierre de la conexión del file descriptor del socket
void cerrarSocket(int sockfd) {
	int retornoClose = close(sockfd);
		if (retornoClose == ERROR) manejarError("Error: No se pudo cerrar la conexión del file descriptor del socket");
}

// ******************************************
// *    Selección entre Sockets Listos	    *
// ******************************************

// select: permite comprobar varios sockets al mismo tiempo. Indica cuáles están listos para leer o para escribir, y cuáles han generado excepciones
void seleccionarSocket(int mayorValorDeFD,
					   fd_set *fdListosParaLectura, fd_set *fdListosParaEscritura, fd_set *fdListosParaEjecucion,
					   int* segundos, int* milisegundos) { // Le indicamos nuestro conjunto de FD de sockets Listos y los parámetros de tiempo

	inicioSeleccion:

	// Si segundos o microsegundos valen NULL, el temporizador nunca expirará y se tendrá que esperar hasta que algún FD esté listo
	if(segundos == NULL || milisegundos == NULL){
		int retornoSelect = select((mayorValorDeFD + 1), fdListosParaLectura, fdListosParaEscritura, fdListosParaEjecucion, NULL);

		if(retornoSelect == ERROR && errno != EINTR) manejarError("Error: No se pudo seleccionar ningún socket del conjunto");

	}else{
		// Si segundos y microsegundos valen cero, select regresará inmediatamente después de interrogar a todos los FD incluidos en los conjuntos de Listos
		struct timeval periodoMaximoDeEspera;

		int seg = *segundos; // segundos
		int microSeg = (*milisegundos) * 1000; // microsegundos

		periodoMaximoDeEspera.tv_sec  = seg;
		periodoMaximoDeEspera.tv_usec = microSeg;

		int selectConLimiteDeTiempo = select((mayorValorDeFD + 1), fdListosParaLectura,
								  fdListosParaEscritura, fdListosParaEjecucion, &periodoMaximoDeEspera);
		if(selectConLimiteDeTiempo == ERROR && errno != EINTR) manejarError("Error: [Timed out] El temporizador ha expirado y no se pudo seleccionar ningún socket");
	}

	if (errno == EINTR){
		errno = 0;

		goto inicioSeleccion;
	}
}

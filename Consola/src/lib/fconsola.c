#include "fconsola.h"

void validar_argumentos(int arg) {
	if(arg != 2) {
		printf("Debe ingresar el archivo a ejecutar como parámetro\n");
		exit(1);
	}
}

void setearValores_config(t_config * archivoConfig){
	puertoNucleo = config_get_int_value(archivoConfig, "PUERTO_NUCLEO");
	ipNucleo = strdup(config_get_string_value(archivoConfig, "IP_NUCLEO"));
}

void leerScript(char * rutaPrograma){
	programa = (string*)malloc(string);

	int _tamanio, descriptorArchivo;
	struct stat infoArchivo; // Ver función 'stat' en stat.h

	descriptorArchivo = open(rutaPrograma, O_RDONLY); // Abre el archivo .asnsisop
		fstat(descriptorArchivo, &infoArchivo); // Obtiene su información
		_tamanio = infoArchivo.st_size;
		programa->tamanio = _tamanio;

		read(descriptorArchivo, programa->cadena, programa->tamanio); // Guardo el script en programa
		close(descriptorArchivo);

		rutaScript = strdup(rutaPrograma); // Extra: se guarda la ruta en una variable global
} // El programa ya está listo para ser enviado a Núcleo

void conectarCon_Nucleo(){
	fd_nucleo = nuevoSocket();
	int ret = conectarSocket(fd_nucleo, ipNucleo, puertoNucleo);
	validar_conexion(ret, 1); // Al ser cliente es terminante
	handshake_cliente(fd_nucleo, "C");

	aplicar_protocolo_enviar(fd_nucleo, ENVIAR_SCRIPT, programa);
	free(programa->cadena);
	free(programa);
}

void liberarRecursos() {
	free(programa->cadena);
	free(programa);
	free(ipNucleo);
	free(rutaScript);
	log_destroy(logger);
	logger = NULL;
}

int validar_servidor(char *id) {
	if(!strcmp(id, "N")) {
		printf("Servidor aceptado\n");
		return TRUE;
	} else {
		printf("Servidor rechazado\n");
		return FALSE;
	}
}
int validar_cliente(char *id) {return 0;}

void esperarMensajesDeNucleo() {
	int head;
	void * mensaje = aplicar_protocolo_recibir(fd_nucleo, &head);
	if (mensaje == NULL) { // desconexión o error
			cerrarSocket(fd_nucleo);
			log_info(logger,"El Núcleo se ha desconectado.");
			free(mensaje);
	}
	else{
		if(head == RECHAZAR_PROGRAMA){
			puts("La UMV no pudo alocar los segmentos pedidos. El programa ha sido rechazado.");
			cerrarSocket(fd_nucleo);
			// log_info(logger, "El sistema ha rechazado al programa %s. Desconectando.", rutaScript);
			free(mensaje);
						}
		else { // se leyó correctamente el mensaje

	while(TRUE) {

		switch(head){

			case IMPRIMIR_TEXTO:{
				/* Incluye 'imprimir', ya que Núcleo le manda la variable "convertida a texto",
				 * asi que ambos casos le llegan como string */
				string* dataAImprimir = (string*)mensaje;
				puts(dataAImprimir->cadena);
				free(mensaje);
			break;
					}
			case FINALIZAR_PROGRAMA:{
				puts("Finalizando el programa.");
					cerrarSocket(fd_nucleo);
					free(mensaje);
			break;
					}
				} // fin del switch-case
			} // fin del while
		}
	}
}

// --LOGGER--
void crearLoggerConsola(){
	char * archivoLogConsola = strdup("CONSOLA_LOG.log");
	logger = log_create("CONSOLA_LOG.log", archivoLogConsola, TRUE, LOG_LEVEL_INFO);
	free(archivoLogConsola);
	archivoLogConsola = NULL;
}

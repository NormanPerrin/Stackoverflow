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
	programa = (string*)malloc(sizeof(string));

	int _tamanio, descriptorArchivo;
	struct stat infoArchivo; // Ver función 'stat' en stat.h

	descriptorArchivo = open(rutaPrograma, O_RDONLY); // Abre el archivo .asnsisop
		fstat(descriptorArchivo, &infoArchivo); // Obtenemos su información
		_tamanio = infoArchivo.st_size;
		programa->tamanio = _tamanio;

		read(descriptorArchivo, programa->cadena, programa->tamanio); // Guardo el script en programa
		close(descriptorArchivo);

		rutaScript = strdup(rutaPrograma); // Extra: se guarda la ruta en una variable global
} // El programa ya está listo para ser enviado a Núcleo

void conectarConNucleo(){
	fd_nucleo = nuevoSocket();
	int ret = conectarSocket(fd_nucleo, ipNucleo, puertoNucleo);
	validar_conexion(ret, 1); // Al ser cliente es terminante
	handshake_cliente(fd_nucleo, "C");

	aplicar_protocolo_enviar(fd_nucleo, ENVIAR_SCRIPT, programa);

	esperar_mensajes();

	cerrarSocket(fd_nucleo);
}

void testLecturaArchivoDeConfiguracion(){
	printf("Puerto Núcleo: %d\n", puertoNucleo);
	printf("IP Núcleo: %s\n", ipNucleo);
}

void liberarRecusos() {
	free(ipNucleo);
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

void esperar_mensajes() {

	int head, tamanioMsj;
	while(TRUE) {
		int ret;
		ret = recibirPorSocket(fd_nucleo, &head, 1);
		validar_recive(ret, 1); // es terminante ya que si hay un error en el recive o desconexión debe terminar

		void * mensaje = aplicar_protocolo_recibir(fd_nucleo, &head, &tamanioMsj);

			if (mensaje == NULL) { // desconexión o error
			cerrarSocket(fd_nucleo);
			log_info(logger,"El Núcleo se ha desconectado");
			break;
			} else { // se leyó correctamente el mensaje

	switch(head){
		case RECHAZAR_PROGRAMA:{
		log_info(logger, "El sistema ha rechazado al programa %s. Desconectando.", rutaScript);
	break;
			}
	case IMPRIMIR:{
		int* valorAImprimir = (int*)mensaje;
		printf("IMPRIMIR: %d\n", *valorAImprimir);
	break;
			}
	case IMPRIMIR_TEXTO:{
		string* textoAImprimir = (string*)mensaje;
		printf("IMPRIMIR: %s\n", textoAImprimir->cadena);
	break;
			}
	case FINALIZAR_PROGRAMA:{

	break;
			}
		}
	}
		free(mensaje);
	}
}

// --LOGGER--
void crearLogger(){
	char * archivoLogConsola = strdup("CONSOLA_LOG.log");
	logger = log_create("CONSOLA_LOG.log", archivoLogConsola, TRUE, LOG_LEVEL_INFO);
	free(archivoLogConsola);
	archivoLogConsola = NULL;
}

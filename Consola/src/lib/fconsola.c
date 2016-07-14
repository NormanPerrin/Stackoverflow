#include "fconsola.h"

// --LOGGER--
void crearLoggerConsola(){
	char * archivoLogConsola = strdup("CONSOLA_LOG.log");
	logger = log_create("CONSOLA_LOG.log", archivoLogConsola, true, LOG_LEVEL_INFO);
	free(archivoLogConsola);
}

void validar_argumentos(int arg) {
	if(arg != 2) {
		printf("Debe ingresar el archivo a ejecutar como parámetro.\n");
		exit(1);
	}
}

void setearValores_config(t_config * archivoConfig){
	puertoNucleo = config_get_int_value(archivoConfig, "PUERTO_NUCLEO");
	ipNucleo = strdup(config_get_string_value(archivoConfig, "IP_NUCLEO"));
}

void leerScript(char * rutaPrograma){

	rutaScript = strdup(rutaPrograma); // Guardo la ruta

	int tamanio, descriptorArchivo;
	struct stat infoArchivo; // función de stat.h

	descriptorArchivo = open(rutaPrograma, O_RDONLY); // Abre el archivo .asnsisop
		if(descriptorArchivo == ERROR) perror("Error al abrir script.");
		fstat(descriptorArchivo, &infoArchivo); // Obtengo la información del script
		tamanio = infoArchivo.st_size;
		programa = malloc(tamanio);
		if(read(descriptorArchivo, programa, tamanio) == ERROR) perror("Error al cerrar script."); // Guardo el script en programa
		close(descriptorArchivo);
} // El programa ya está listo para ser enviado a Núcleo

void conectarCon_Nucleo(){
	fd_nucleo = nuevoSocket();
	int ret = conectarSocket(fd_nucleo, ipNucleo, puertoNucleo);
	validar_conexion(ret, 1); // Al ser cliente es terminante
	handshake_cliente(fd_nucleo, "C");
}

void liberarRecursos() {
	free(programa);
	free(ipNucleo);
	free(rutaScript);
	log_destroy(logger);
	logger = NULL;
}

int validar_servidor(char *id) {
	if(!strcmp(id, "N")) {
		printf("Servidor aceptado.\n");
		return TRUE;
	} else {
		printf("Servidor rechazado.\n");
		return FALSE;
	}
}
int validar_cliente(char *id) {return 0;}

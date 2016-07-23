#include "fconsola.h"

// --LOGGER--
void crearLoggerConsola(){
	char * archivoLogConsola = strdup("CONSOLA_LOG.log");
	logger = log_create("CONSOLA_LOG.log", archivoLogConsola, true, LOG_LEVEL_INFO);
	free(archivoLogConsola); archivoLogConsola = NULL;
}

void validar_argumentos(int arg) {
	if(arg != 2) {
		printf("Error al iniciar: Debe ingresar el script como parámetro.\n");
		exit(1);
	}
}

void setearValores_config(t_config * archivoConfig){
	puertoNucleo = config_get_int_value(archivoConfig, "PUERTO_NUCLEO");
	ipNucleo = strdup(config_get_string_value(archivoConfig, "IP_NUCLEO"));
}

void leerScript(char * rutaPrograma){

	int tamanio, descriptorArchivo;
	struct stat infoArchivo; // función de stat.h

	descriptorArchivo = open(rutaPrograma, O_RDONLY); // Abre el archivo .asnsisop
	if(descriptorArchivo == ERROR) perror("Error al abrir script.");

	fstat(descriptorArchivo, &infoArchivo); // Obtengo la información del script

	tamanio = infoArchivo.st_size;
	programa = malloc(tamanio);

	tamanioPrograma = read(descriptorArchivo, programa, tamanio); // Guardo el script en programa
	if(tamanioPrograma == ERROR) perror("Error al cerrar script.");
	programa = realloc(programa, tamanioPrograma);

	close(descriptorArchivo);
} // El programa ya está listo para ser enviado a Núcleo

void conectarCon_Nucleo(){
	fd_nucleo = nuevoSocket();
	int ret = conectarSocket(fd_nucleo, ipNucleo, puertoNucleo);
	validar_conexion(ret, 1); // Al ser cliente es terminante
	handshake_cliente(fd_nucleo, "C");
}

void exitConsola(){
	cerrarSocket(fd_nucleo);
	liberarRecursos(); // Libera memoria asignada
	printf("Cerrando proceso Consola.\n");
}

void liberarRecursos() {
	free(programa); programa = NULL;
	free(ipNucleo); ipNucleo = NULL;
	log_destroy(logger); logger = NULL;
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

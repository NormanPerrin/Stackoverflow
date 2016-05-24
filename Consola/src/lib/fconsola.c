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

void leerScript(char * rutaScript){
	nombreScript = (t_string*)malloc(t_string);
	(*nombreScript).tamanio = strlen(rutaScript) + 1;
	(*nombreScript).texto = strdup(rutaScript);
}

void conectarConNucleo(){
	fd_nucleo = nuevoSocket();
	int ret = conectarSocket(fd_nucleo, ipNucleo, puertoNucleo);
	validar_conexion(ret, 1); // Al ser cliente es terminante
	handshake_cliente(fd_nucleo, "C");

	aplicar_protocolo_enviar(fd_nucleo, ENVIAR_SCRIPT, nombreScript);

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


void enviar_script(char *ruta) {

	// msg_t *msg_to_send;
	// msg_to_send = aplicar_protocolo_enviar(ENVIAR_SCRIPT, ruta);
	// enviarPorSocket(fd_nucleo, msg_to_send, sizeof(msg_to_send));
}


void esperar_mensajes() {

	uint8_t *head = (uint8_t*)reservarMemoria(1); // 0 .. 255

	while(TRUE) {
		int ret;
		ret = recibirPorSocket(fd_nucleo, head, 1);
		validar_recive(ret, 1); // es terminante ya que si hay un error en el recive o desconexión debe terminar
		aplicar_protocolo_recibir(fd_nucleo, *head);
	}

	free(head);
}


void imprimir(char *texto) { printf("IMPRIMIR: %s\n", texto); }

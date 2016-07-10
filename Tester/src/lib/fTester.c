#include "fTester.h"

void mostrar_protocolos() {
	printf("\nIMPRIMIR = %d\n"
				"IMPRIMIR_TEXTO = %d\n"
				"LEER_BYTES = %d\n"
				"INICIAR_PROGRAMA = %d\n"
				"ESCRIBIR_BYTES = %d\n"
				"FINALIZAR_PROGRAMA = %d\n"
				"ENVIAR_SCRIPT = %d\n"
				"RESPUESTA_PEDIDO = %d\n"
				"LEER_PAGINA = %d\n"
				"ESCRIBIR_PAGINA = %d\n"
				"DEVOLVER_PAGINA = %d\n"
				"ENVIAR_PCB = %d\n"
				"RESPUESTA_INICIO_PROGRAMA = %d\n\n",
				IMPRIMIR,
				IMPRIMIR_TEXTO,
				INICIAR_PROGRAMA,
				LEER_PAGINA,
				ESCRIBIR_PAGINA,
				FINALIZAR_PROGRAMA,
				ENVIAR_SCRIPT,
				RESPUESTA_PEDIDO,
				LEER_PAGINA,
				ESCRIBIR_PAGINA,
				DEVOLVER_PAGINA,
				PCB,
				RESPUESTA_INICIO_PROGRAMA
				);
}

void validar_argumentos(int argc) {

	if(argc != 3) {
		printf("./Tester PUERTO MODO[0 servidor, 1 cliente]\n");
		exit(1);
	}

}


void testear(char* arg_puerto, char* arg_modo) {

	int puerto = atoi(arg_puerto);
	int modo = '0' - arg_modo[0];
	int sock = nuevoSocket();
	char *msg = (char*)reservarMemoria(CHAR*2);

	if(modo == 0) { // -- SERVIDOR --

		asociarSocket(sock, puerto);
		escucharSocket(sock, 1);

		int cliente = aceptarConexionSocket(sock);

		printf("\nIngresar handshake: ");
		scanf("%s", msg);
		msg[1] = '\0';
		handshake_servidor(cliente, msg);

		int ret;
		while(TRUE) {

			printf("0 para recibir, 1 para enviar, 2 para mostrar protocolos \n>");
			scanf("%s", msg);
			msg[1] = '\0';
			if( msg[0] == '0') {

				while(TRUE) {
				ret = recibirPorSocket(cliente, msg, CHAR*2);
				validar_recive(ret, 1);
				msg[1] = '\0';
				printf("%s\n", msg);
				}

			} else if ( msg[1] == '1' ) {

				// ads

			} else {
				mostrar_protocolos();
			}

		}

	} else { // -- CLIENTE --

		int ret = conectarSocket(sock, "127.0.0.1", puerto);
		validar_conexion(ret, 1);

		printf("Ingresar handshake: ");
		scanf("%s", msg);
		msg[1] = '\0';
		handshake_cliente(sock, msg);

		while(TRUE) {

			printf("0 para recibir, 1 para enviar, 2 para mostrar protocolos \n>");
			scanf("%s", msg);
			msg[1] = '\0';
			if( msg[0] == '0') {

				while(TRUE) {
					ret = recibirPorSocket(sock, msg, CHAR*2);
					validar_recive(ret, 0);
					msg[1] = '\0';
					printf("%s\n", msg);
				}

			} else if ( msg[1] == '1' ) {

				// asd

			} else {
				mostrar_protocolos();
			}
		}
	}

	free(msg);
}

void setearValores_config(t_config * archivoConfig) {NULL;}
int validar_cliente(char *id) {return TRUE;}
int validar_servidor(char *id) {return TRUE;}

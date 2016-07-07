#include "secundariasCPU.h"

// Funciones
void setearValores_config(t_config * archivoConfig){
	config = (t_configuracion*)reservarMemoria(sizeof(t_configuracion));
	config->ipNucleo = strdup(config_get_string_value(archivoConfig, "IP_NUCLEO"));
	config->ipUMC = strdup(config_get_string_value(archivoConfig, "IP_UMC"));
	config->puertoNucleo = config_get_int_value(archivoConfig, "PUERTO_NUCLEO");
	config->puertoUMC = config_get_int_value(archivoConfig, "PUERTO_UMC");
}

void recibirYvalidarEstadoDelPedidoAUMC(){

	int head;
	void* entrada = NULL;
	int* estadoDelPedido = NULL;

	entrada = aplicar_protocolo_recibir(fdUMC, &head);

	if(head == RESPUESTA_PEDIDO){
			estadoDelPedido = (int*)entrada;
				free(entrada);

	 if(*estadoDelPedido == NO_PERMITIDO){
			printf("UMC ha rechazado un pedido de lectura/escritura del proceso #%d\n", pcbActual->pid);
				free(estadoDelPedido);

			int* _pid = malloc(INT);
			*_pid= pcbActual->pid;

			aplicar_protocolo_enviar(fdNucleo, ABORTO_PROCESO, _pid);
			free(_pid);
			printf("Finalizando ejecución del programa actual.\n");
				liberarPcbActiva();
			printf("Esperando nuevo proceso.\n");
		 }
	}
		free(estadoDelPedido);
}

void liberarPcbActiva(){
	free(pcbActual->indiceCodigo);
	free(pcbActual->indiceEtiquetas);
	free(pcbActual->indiceStack->posicionesArgumentos);
	free(pcbActual->indiceStack->variables);
	free(pcbActual->indiceStack);

	free(pcbActual);
	pcbActual = NULL;
}

int validar_servidor(char *id) {
	if( !strcmp(id, "U") || !strcmp(id, "N") ) {
		printf("Servidor aceptado\n");
		return TRUE;
	} else {
		printf("Servidor rechazado\n");
		return FALSE;
	}
}

int validar_cliente(char *id) {return 0;}

char* charToString(char c) {
	char* caracter = malloc(2);
	*caracter = c;
	*(caracter + 1) = '\0';
	return caracter;
}

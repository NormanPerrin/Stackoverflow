#include "fumc.h"

// Globales
int exitFlag = FALSE; // exit del programa

void setearValores_config(t_config * archivoConfig) {
	config = (t_configuracion*)reservarMemoria(sizeof(t_configuracion));
	config->ip_swap = (char*)reservarMemoria(CHAR*20);
	config->backlog = config_get_int_value(archivoConfig, "BACKLOG");
	config->puerto = config_get_int_value(archivoConfig, "PUERTO");
	config->ip_swap = strdup(config_get_string_value(archivoConfig, "IP_SWAP"));
	config->puerto_swap = config_get_int_value(archivoConfig, "PUERTO_SWAP");
	config->marcos = config_get_int_value(archivoConfig, "MARCOS");
	config->marco_size = config_get_int_value(archivoConfig, "MARCO_SIZE");
	config->marco_x_proceso = config_get_int_value(archivoConfig, "MARCO_X_PROC");
	config->entradas_tlb = config_get_int_value(archivoConfig, "ENTRADAS_TLB");
	config->retardo = config_get_int_value(archivoConfig, "RETARDO");
}


void conectarConSwap(){
	sockClienteDeSwap = nuevoSocket();
	int ret = conectarSocket(sockClienteDeSwap, config->ip_swap, config->puerto_swap);
	validar_conexion(ret, 1);
	handshake_cliente(sockClienteDeSwap, "U");
}


void crearHilos() {
	pthread_t hilo_servidor, hilo_consola;
	pthread_create(&hilo_servidor, NULL, (void*)servidor, NULL);
	pthread_create(&hilo_consola, NULL, (void*)consola, NULL);
	pthread_join(hilo_consola, NULL);
	pthread_join(hilo_servidor, NULL);
}


void servidor() {

	sockServidor = nuevoSocket();
	asociarSocket(sockServidor, config->puerto);
	escucharSocket(sockServidor, config->backlog);

	while(!exitFlag) {

		int ret_handshake = 0;
		int *sockCliente = (int*)reservarMemoria(INT);

		while(ret_handshake == 0) {

			*sockCliente = aceptarConexionSocket(sockServidor);
			if( validar_conexion(*sockCliente, 0) == FALSE ) {
				continue;
			} else {
				ret_handshake = handshake_servidor(*sockCliente, "U");
				enviarPorSocket(*sockCliente,(int*)config->marco_size,INT);
			}

		}

		crearHiloCliente(sockCliente);
	}

	close(sockServidor);
}


void consola() {

	printf("Hola! Ingresá \"#\" para salir\n");
	char *mensaje = reservarMemoria(CHAR*20);

	while(!exitFlag) {

		scanf("%[^\n]%*c", mensaje);

		if(!strcmp(mensaje, "#")) {
			printf("Saliendo de UMC\n");
			exitFlag = TRUE;
		} else {
			printf("Consola: %s\n", mensaje);
		}

	}

	free(mensaje);
}


void crearHiloCliente(int *sockCliente) {
	pthread_t hilo_cliente;
	pthread_create(&hilo_cliente, NULL, (void*)cliente, sockCliente);
}

void cliente(void* fdCliente) {

	int sockCliente = *((int*)fdCliente);
	uint8_t *head = (uint8_t*)reservarMemoria(1); // 0 .. 255
	int ret = 1;

	while(ret > 0 && !exitFlag) {

		ret = recibirPorSocket(sockCliente, head, 1);

		if( validar_recive(ret, 0) == FALSE) { // Si se desconecta una CPU o hay error en mensaje no pasa nada. Por eso no terminante
			break;
		} else {
			aplicar_protocolo_recibir(sockCliente, *head);
		}

	}

	free(head);
	close(sockCliente);
}


void liberarEstructura() {
	free(config->ip_swap);
	free(config);
}

void liberarRecusos() {
	// liberar otros recursos
	free(memoria);
	free(tabla_paginas);
	free(tlb);
	sem_destroy(&mutex);
	liberarEstructura();
}


int validar_cliente(char *id) {
	if( !strcmp(id, "N") || !strcmp(id, "P") ) {
		printf("Cliente aceptado\n");
		return TRUE;
	} else {
		printf("Cliente rechazado\n");
		return FALSE;
	}
}

int validar_servidor(char *id) {
	if(!strcmp(id, "S")) {
		printf("Servidor aceptado\n");
		return TRUE;
	} else {
		printf("Servidor rechazado\n");
		return FALSE;
	}
}


void iniciarEstructuras() {

	sem_init(&mutex, 0, 1);

	int length = config->marcos * config->marco_size;
	memoria = reservarMemoria(length); // Google Chrome be like
	memcpy(memoria, "", length);

	length = config->marcos * sizeof(tp_t);
	tabla_paginas = reservarMemoria(length);
	iniciarTP();

	length = config->entradas_tlb * sizeof(tlb_t);
	tlb = reservarMemoria(length);

}


void iniciarTP() {

	int marco;
	sem_wait(&mutex);
	for(marco = 0; marco < entradas_tp; marco++) reset_entrada(marco);
	sem_post(&mutex);

	entradas_tp = config->marcos;
}


void reset_entrada(int pos) {
	tabla_paginas[pos].pagina = -1;
	tabla_paginas[pos].pid = -1;
	tabla_paginas[pos].marco = -1;
	tabla_paginas[pos].bit_uso = 0;
	tabla_paginas[pos].bit_modificado = 0;
	tabla_paginas[pos].bit_presencia = 0;
}


int inciar_programa(int pid, int paginas) {

	agregar_tp(pid, paginas);

	iniciar_programa_t *arg = reservarMemoria(sizeof(iniciar_programa_t));
	aplicar_protocolo_enviar(sockClienteDeSwap, INICIAR_PROGRAMA, (void*)arg);
	int ret = *( (int*)aplicar_protocolo_recibir(sockClienteDeSwap, RESPUESTA_PEDIDO) );

	free(arg);
	return ret;
}


void agregar_tp(int pid, int paginas) {

	sem_wait(&mutex);

	int i, pos = 0;
	for(i = 0; i < paginas; i++) {

		while( tabla_paginas[pos].pid != -1 ) {

			if( pos == entradas_tp ) {
				entradas_tp += 20; // 20 para no hacer realloc tan seguido
				realloc(tabla_paginas, entradas_tp * sizeof(tp_t));
				int i;
				for(i = pos; i < entradas_tp; i++) {
					reset_entrada(i);
				}
			}

			pos++;
		}

		tabla_paginas[pos].pagina = i;
		tabla_paginas[pos].pid = pid;

	}

	sem_post(&mutex);

}


void eliminar_pagina(int pid, int pagina) {

	sem_wait(&mutex);

	int pos = buscar_pagina(pid, pagina);
	if(pos == ERROR) {
		fprintf(stderr, "Error: referencia de pid: %d a página: %d no encontrada\n", pid, pagina);
	} else {
		reset_entrada(pos);
	}

	sem_post(&mutex);
}


int buscar_pagina(int pid, int pagina) {

	sem_wait(&mutex);

	int pos = 0;
	while(pos < entradas_tp) {

		if( (tabla_paginas[pos].pid == pid) && (tabla_paginas[pos].pagina == pagina) )
			return pos;

		pos++;
	}

	sem_post(&mutex);

	return ERROR;

}


void *leer_bytes(int pid, int pagina, int offset, int tamanio) {

	void *contenido = reservarMemoria(tamanio);

	int pos = buscar_pagina(pid, pagina);
	if(pos == ERROR) {
		return NULL;
	}

	int dir_fisica;
	dir_fisica = (pos * config->marco_size) + offset;
	sem_wait(&mutex);
	memcpy(contenido, (memoria + dir_fisica), tamanio);
	sem_post(&mutex);

	return contenido;

}


int escribir_bytes(int pid, int pagina, int offset, int tamanio, void *contenido) {

	int pos = buscar_pagina(pid, pagina);
	if(pos == ERROR) return ERROR;

	int dir_fisica = (pos * config->marco_size) + offset;
	sem_wait(&mutex);
	memcpy(memoria + dir_fisica, contenido, tamanio);
	sem_post(&mutex);

	return TRUE;

}


void finalizar_programa(int pid) {

	int pos;

	sem_wait(&mutex);

	for(pos = 0; pos < entradas_tp; pos++) {
		if(tabla_paginas[pos].pid == pid) {
			reset_entrada(pos);
		}
	}

	sem_post(&mutex);
}


void pedir_pagina(int fd, int pid, int pagina) {

	iniciar_programa_t *arg;
	arg->pid = pid;
	arg->paginas = pagina;
	aplicar_protocolo_enviar(sockClienteDeSwap, LEER_PAGINA, arg);

	void *contenido;
	contenido = aplicar_protocolo_recibir(sockClienteDeSwap, RESPUESTA_PEDIDO);
	if(contenido == NULL) {
		aplicar_protocolo_enviar(fd, RESPUESTA_PEDIDO, NULL);
	}
}

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

// <CONEXIONES>

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


void consola() {

	printf("Hola! Ingresá \"salir\" para salir\n");
	char *mensaje = reservarMemoria(CHAR * MAX_CONSOLA);

	while(!exitFlag) {

		scanf("%[^\n]%*c", mensaje);

		if(!strcmp(mensaje, "salir")) {
			printf("Saliendo de UMC\n");
			exitFlag = TRUE;
		} else {
			printf("Consola: %s\n", mensaje);
		}

	}

	free(mensaje);
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
				continue; // vuelve al inicio del while
			} else {
				ret_handshake = handshake_servidor(*sockCliente, "U");
				enviarTamanioMarco(*sockCliente, config->marco_size);
			}

		} // cuando sale hay un cliente válido

		crearHiloCliente(sockCliente);

	} // cuando sale indicaron cierre del programa

	close(sockServidor);
}


void crearHiloCliente(int *sockCliente) {
	pthread_t hilo_cliente;
	pthread_create(&hilo_cliente, NULL, (void*)cliente, sockCliente);
}


void cliente(void* fdCliente) {

	int sockCliente = *((int*)fdCliente);
	free(fdCliente);

	uint16_t *head = (uint16_t*)reservarMemoria(INT);
	int ret = 1;

	while(ret > 0 && !exitFlag) {

		ret = recibirPorSocket(sockCliente, head, 2); // recibe 16 bits = 2 bytes (uint16_t)

		if( validar_recive(ret, 0) == FALSE) { // Si se desconecta una CPU o hay error en mensaje no pasa nada. Por eso no terminante
			break;
		} else {
			void *mensaje = aplicar_protocolo_recibir(sockCliente, *head); // recibo mensaje
			void (*funcion)(int, void*) = elegirFuncion(*head); // elijo función a ejecutar según protocolo
			funcion(sockCliente, mensaje); // ejecuto función
			free(mensaje); // aplicar_protocolo_recibir pide memoria, por lo tanto hay que liberarla
		}

	}

	free(head);
	close(sockCliente);
}

// </CONEXIONES>


// <PRINCIPAL>

void inciar_programa(int fd, void *msj) {

	inciarPrograma_t *mensaje = (inciarPrograma_t*)msj; // casteo

	// 1) Agrego a tabla de páginas
	agregar_tp(mensaje->pid, mensaje->paginas);

	// 2) Envío a Swap
	aplicar_protocolo_enviar(sockClienteDeSwap, INICIAR_PROGRAMA, msj);

	// 3) Espero respuesta de Swap
	int *respuesta = (int*)aplicar_protocolo_recibir(sockClienteDeSwap, RESPUESTA_PEDIDO);

	// 4) Respondo a Núcleo como salió la operación
	responder(sockClienteDeSwap, *respuesta);

}

void leer_bytes(int fd, void *msj) {

	leerBytes_t *mensaje = (leerBytes_t*)msj; // casteo

	// veo cual es el pid activo de esta CPU
	int pos = buscarPosPid(fd);
	int pid = pids[pos].pid;

	// busco el marco de la página en TLB TP y Swap
	int marco = buscarPagina(fd, pid, mensaje->pagina);

	// actualizo TLB y TP
	actualizar_tlb(pid, mensaje->pagina, marco);
	actualizar_tp(pid, mensaje->pagina, -1, -1, 1);

	// busco el código que me piden
	void *contenido = reservarMemoria(mensaje->tamanio);
	int pos_real = marco * (config->marco_size) + mensaje->offset;
	memcpy(contenido, memoria + pos_real, mensaje->tamanio);

	// devuelvo el contenido solicitado
	aplicar_protocolo_enviar(fd, LEER_PAGINA, contenido);

	free(contenido);
}

void escribir_bytes(int fd, void *msj) {

	escribirBytes_t *mensaje = (escribirBytes_t*)msj; // casteo

	// veo cual es el pid activo de esta CPU
	int pos = buscarPosPid(fd);
	int pid = pids[pos].pid;

	// busco el marco de la página en TLB TP y Swap
	int marco = buscarPagina(fd, pid, mensaje->pagina);

	// actualizo TLB y TP
	actualizar_tlb(pid, mensaje->pagina, marco);
	actualizar_tp(pid, mensaje->pagina, -1, 1, 1);

	// escribo contenido
	int pos_real = marco * (config->marco_size) + mensaje->offset;
	memcpy(memoria + pos_real, mensaje->contenido, mensaje->tamanio);
}

void finalizar_programa(int fd, void *msj) {

	finalizarPrograma_t *mensaje = (finalizarPrograma_t*)msj;

	// aviso a Swap
	int *pid;
	*pid = mensaje->pid;
	aplicar_protocolo_enviar(sockClienteDeSwap, FINALIZAR_PROGRAMA, pid);

	// recorro páginas de pid
	int paginas = contar_paginas(*pid), i;
	for(i = 0; i < paginas; i++) {

		int pos = buscar_tp(*pid, i);
		if(tabla_paginas[pos].bit_presencia == 1) { // elimino página de MP
			int pos_real = (tabla_paginas[pos].marco) * (config->marco_size);
			memset(memoria + pos_real, '\0', config->marco_size);
		}

		// elimino página de TP
		eliminar_pagina(*pid, i);

		// elimino página de TLB si está
		borrar_tlb(*pid, i);

	}

}

// | int pid |
void cambiarPid(int fd, void *mensaje) {
	int *pid = (int*)mensaje;
	actualizarPid(fd, *pid);
}

// </PRINCIPAL>


// <TABLA_PAGINA>

void iniciarTP() {
	entradas_tp = config->marcos;
	int marco;
	for(marco = 0; marco < entradas_tp; marco++) reset_entrada(marco);
}

void reset_entrada(int pos) {
	sem_wait(&mutex);
	tabla_paginas[pos].pagina = -1;
	tabla_paginas[pos].pid = -1;
	tabla_paginas[pos].marco = -1;
	tabla_paginas[pos].bit_uso = 0;
	tabla_paginas[pos].bit_modificado = 0;
	tabla_paginas[pos].bit_presencia = 0;
	sem_post(&mutex);
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

int contar_paginas(int pid) {

	int cont = 0;
	int i = 0;
	while(i < entradas_tp) {
		if(tabla_paginas[i].pid == pid)
			cont++;
		i++;
	}

	return cont;
}

void eliminar_pagina(int pid, int pagina) {

	sem_wait(&mutex);

	int pos = buscar_pagina(pid, pagina);
	if(pos == ERROR)
		fprintf(stderr, "Error: referencia de pid: %d a página: %d no encontrada\n", pid, pagina);
	else
		reset_entrada(pos);

	sem_post(&mutex);
}

int buscar_tp(int pid, int pagina) {

	sem_wait(&mutex);

	int pos = 0;
	while(pos < entradas_tp) {

		if( (tabla_paginas[pos].pid == pid) && (tabla_paginas[pos].pagina == pagina) ) {
			sem_post(&mutex);
			return pos;
		}
		pos++;
	}

	sem_post(&mutex);

	return ERROR;

}

int buscarPagina(int fd, int pid, int pagina) {

	int marco;

	// 1) busco en TLB
	marco = buscar_tlb(pid, pagina);
	if( marco != ERROR ) { // TLB Hit

	} else { // TLB fault

		// 2) busco en TP
		int pos_tp = buscar_tp(pid, pagina);
		if(pos_tp == ERROR) // no se inicializó el proceso
			responder(fd, FALSE);

		// veo si está en MP y si no encuentro cargo desde Swap
		if( tabla_paginas[pos_tp].bit_presencia == 0 ) // page fault
				marco = pedir_pagina_swap(fd, pid, pagina);
		else // tp hit
			marco = tabla_paginas[pos_tp].marco;
	}

	return marco;
}

int cargar_pagina(int pid, int pagina, void *contenido) {
	return 0;
}

void actualizar_marco_tp(int pos, int valor) {
	tabla_paginas[pos].marco = valor;
}

void actualizar_presencia_tp(int pos, int valor) {
	tabla_paginas[pos].bit_presencia = valor;
}

void actualizar_modificado_tp(int pos, int valor) {
	tabla_paginas[pos].bit_modificado = valor;
}

void actualizar_uso_tp(int pos, int valor) {
	tabla_paginas[pos].bit_uso = valor;
}

// </TABLA_PAGINA>


// <AUXILIARES>

void iniciarEstructuras() {

	sem_init(&mutex, 0, 1);

	int sizeof_memoria = config->marcos * config->marco_size;
	memoria = reservarMemoria(sizeof_memoria); // Google Chrome be like

	memset(memoria, '\0', sizeof_memoria);

	int sizeof_tp = config->marcos * sizeof(tp_t);
	tabla_paginas = reservarMemoria(sizeof_tp);
	iniciarTP();

	int sizeof_tlb = config->entradas_tlb * sizeof(tlb_t);
	tlb = reservarMemoria(sizeof_tlb);

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

void enviarTamanioMarco(int fd, int tamanio) {
	int *msj = (int*)reservarMemoria(INT);
	*msj = tamanio;
	enviarPorSocket(fd, msj, INT);
	free(msj);
}

void *elegirFuncion(protocolo head) {

	switch(head) {

		case INICIAR_PROGRAMA:
			return inciar_programa;

		case LEER_PAGINA:
			return leer_bytes;

		case ESCRIBIR_PAGINA:
			return escribir_bytes;

		case FINALIZAR_PROGRAMA:
			return finalizar_programa;

		case INDICAR_PID:
			return cambiarPid;

		default:
			fprintf(stderr, "No existe protocolo definido para %d\n", head);
			break;

	}

	return NULL;
}

void responder(int fd, int respuesta) {
	int *resp;
	*resp = respuesta;
	aplicar_protocolo_enviar(fd, RESPUESTA_PEDIDO, resp);
}

int buscarPosPid(int fd) {

	int i = 0;
	while(i < MAX_CONEXIONES) {
		if(pids[i].fd == fd) return i;
		i++;
	}

	return ERROR;
}

void actualizarPid(int fd, int pid) {
	int pos = buscarPosPid(fd);
	pids[pos].pid = pid;
}

int pedir_pagina_swap(int fd, int pid, int pagina) {

	int marco = ERROR;

	// pido página a Swap
	pedidoPagina_t pedido;
	pedido.pid = pid;
	pedido.pagina = pagina;
	aplicar_protocolo_enviar(sockClienteDeSwap, LEER_PAGINA, &pedido);

	// espero respuesta de Swap
	void *contenido_pagina = aplicar_protocolo_recibir(sockClienteDeSwap, LEER_PAGINA);
	if(contenido_pagina == NULL) // no encontró la página o hubo un fallo
		responder(fd, FALSE);
	else {// tengo que cargar la página a MP
		marco = cargar_pagina(pid, pagina, contenido_pagina);
	}

	return marco;
}

// </AUXILIARES>

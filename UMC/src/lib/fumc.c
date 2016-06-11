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

// <CONEXIONES_FUNCS>

void conectarConSwap() {
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

	close(sockClienteDeSwap);
	close(sockServidor);
}


void crearHiloCliente(int *sockCliente) {
	pthread_t hilo_cliente;
	pthread_create(&hilo_cliente, NULL, (void*)cliente, sockCliente);
}


void cliente(void* fdCliente) {

	int sockCliente = *((int*)fdCliente);
	free(fdCliente);

	int *head = (int*)reservarMemoria(INT);
	int ret = 1;

	while(ret > 0 && !exitFlag) {
		void *mensaje = aplicar_protocolo_recibir(sockCliente, head); // recibo mensaje
		void (*funcion)(int, void*) = elegirFuncion(*head); // elijo función a ejecutar según protocolo
		sem_wait(&mutex);
		funcion(sockCliente, mensaje); // ejecuto función
		sem_post(&mutex);
		free(mensaje); // aplicar_protocolo_recibir pide memoria, por lo tanto hay que liberarla
	}

	free(head);
	close(sockCliente);
}

int pedir_pagina_swap(int fd, int pid, int pagina) {

	int marco = ERROR;

	// pido página a Swap
	pedidoPagina_t pedido;
	pedido.pid = pid;
	pedido.pagina = pagina;
	aplicar_protocolo_enviar(sockClienteDeSwap, LEER_PAGINA, &pedido);

	// espero respuesta de Swap
	int *protocolo = (int*)reservarMemoria(INT);
	void *contenido_pagina = aplicar_protocolo_recibir(sockClienteDeSwap, protocolo);
	if(contenido_pagina == NULL || *protocolo != DEVOLVER_PAGINA) {// no encontró la página o hubo un fallo
		// seteo mensaje de respuesta
		respuestaPedido *respuesta = NULL; // TODO
		aplicar_protocolo_enviar(fd, RESPUESTA_PEDIDO, respuesta);
	}
	else // tengo que cargar la página a MP
		marco = cargar_pagina(pid, pagina, contenido_pagina);

	free(protocolo);
	return marco;
}

void enviarTamanioMarco(int fd, int tamanio) {
	int *msj = (int*)reservarMemoria(INT);
	*msj = tamanio;
	enviarPorSocket(fd, msj, INT);
	free(msj);
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

// </CONEXIONES_FUNCS>


// <PRINCIPAL>

void inciar_programa(int fd, void *msj) {

	inciarPrograma_t *mensaje = (inciarPrograma_t*)msj; // casteo

	// 1) Agrego a tabla de páginas
	agregar_paginas_nuevas(mensaje->pid, mensaje->paginas);

	// 2) Envío a Swap
	aplicar_protocolo_enviar(sockClienteDeSwap, INICIAR_PROGRAMA, msj);

	// 3) Espero respuesta de Swap
	int *protocolo = (int*)reservarMemoria(INT);
	respuestaInicioPrograma *respuesta = (respuestaInicioPrograma*)aplicar_protocolo_recibir(sockClienteDeSwap, protocolo);
	compararProtocolos(*protocolo, RESPUESTA_INICIO_PROGRAMA); // comparo protocolo recibido con esperado
	free(protocolo);

	// 4) Respondo a Núcleo como salió la operación
	aplicar_protocolo_enviar(fd, RESPUESTA_INICIO_PROGRAMA, respuesta); // todo: ver

}

void leer_bytes(int fd, void *msj) {

	leerBytes_t *mensaje = (leerBytes_t*)msj; // casteo

	// veo cual es el pid activo de esta CPU
	int pos = buscarPosPid(fd);
	int pid = pids[pos].pid;

	// busco el marco de la página en TLB TP y Swap
	int marco = buscarPagina(fd, pid, mensaje->pagina);

	// actualizo TLB y TP
	buscar_tlb(pid, mensaje->pagina); // la función buscar actualiza referencia también
	actualizar_tp(pid, mensaje->pagina, marco, 1, -1, 1);

	// busco el código que me piden
	void *contenido = reservarMemoria(mensaje->tamanio);
	int pos_real = marco * (config->marco_size) + mensaje->offset;
	memcpy(contenido, memoria + pos_real, mensaje->tamanio);

	// devuelvo el contenido solicitado
	aplicar_protocolo_enviar(fd, DEVOLVER_CONTENIDO, contenido);

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
	buscar_tlb(pid, mensaje->pagina); // la fución buscar actualiza referencia también
	actualizar_tp(pid, mensaje->pagina, marco, 1, 1, 1);

	// escribo contenido
	int pos_real = marco * (config->marco_size) + mensaje->offset;
	memcpy(memoria + pos_real, mensaje->contenido, mensaje->tamanio);

}

void finalizar_programa(int fd, void *msj) {

	finalizarPrograma_t *mensaje = (finalizarPrograma_t*)msj;

	// aviso a Swap
	int *pid = NULL;
	*pid = mensaje->pid;
	aplicar_protocolo_enviar(sockClienteDeSwap, FINALIZAR_PROGRAMA, pid);

	// recorro páginas de pid
	int pos = pos_pid(*pid);
	int i;

	for(i = 0; i < tabla_paginas[pos]->paginas; i++) {

		if(tabla_paginas[pos]->tabla[i].bit_presencia == 1) // elimino página de MP
			borrarMarco(tabla_paginas[pos]->tabla[i].marco);

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
	int i;
	for(i = 0; i < MAX_PROCESOS; i++)
		reset_entrada(i);
}

void reset_entrada(int pos) {

	subtp_t set;
	set.pagina = -1;
	set.marco = -1;
	set.bit_presencia = 0;
	set.bit_uso = 0;
	set.bit_modificado = 0;

	int subpos = 0;
	while(subpos < tabla_paginas[pos]->paginas) {
		setear_entrada(pos, subpos, set);
		subpos++;
	}
}

void setear_entrada(int pos, int subpos, subtp_t set) {
	tabla_paginas[pos]->tabla[subpos].pagina = set.pagina;
	tabla_paginas[pos]->tabla[subpos].marco = set.marco;
	tabla_paginas[pos]->tabla[subpos].bit_presencia = set.bit_presencia;
	tabla_paginas[pos]->tabla[subpos].bit_uso = set.bit_uso;
	tabla_paginas[pos]->tabla[subpos].bit_modificado = set.bit_modificado;
}

int pos_pid(int pid) {

	int pos = 0;
	while(pos < MAX_PROCESOS) {
		if(tabla_paginas[pos]->pid == pid)
			return pos;
		pos++;
	}

	return ERROR;
}

void agregar_paginas_nuevas(int pid, int paginas) {

	int i;
	for(i = 0; i < paginas; i++)
		agregar_pagina_nueva(pid, i);
}

void agregar_pagina_nueva(int pid, int pagina) {

	int p = pos_pid(pid);
	subtp_t set;
	set.pagina = pagina;
	set.marco = -1;
	set.bit_presencia = 0;
	set.bit_modificado = 0;
	set.bit_uso = 0;
	setear_entrada(p, pagina-1, set);
}


int contar_paginas_asignadas(int pid) {

	int paginas_asignadas = 0;
	int pos = pos_pid(pid);

	int i = 0;
	while(i < tabla_paginas[pos]->paginas) {
		if(tabla_paginas[pos]->tabla[i].bit_presencia == 1)
			paginas_asignadas++;
		i++;
	}

	return paginas_asignadas;
}

void eliminar_pagina(int pid, int pagina) {

	int pos = pos_pid(pid);
	if(pos == ERROR)
		fprintf(stderr, "Error: <eliminar_pagina> referencia de pid: %d a página: %d no encontrada\n", pid, pagina);
	else {
		subtp_t set;
		set.pagina = -1;
		set.marco = -1;
		set.bit_uso = 0;
		set.bit_presencia = 0;
		set.bit_modificado = 0;
		setear_entrada(pos, pagina-1, set);
	}
}

int buscarPagina(int fd, int pid, int pagina) {

	int marco;

	// 1) busco en TLB
	marco = buscar_tlb(pid, pagina);
	if( marco != ERROR ) { // TLB Hit

	} else { // TLB fault

		// 2) busco en TP
		int pos_tp = pos_pid(pid);
		if(pos_tp == ERROR) {// no se inicializó el proceso
			// setear respuesta
			respuestaPedido *respuesta = NULL;
			aplicar_protocolo_enviar(fd, RESPUESTA_PEDIDO, respuesta); // todo
		}

		// veo si está en MP y si no encuentro cargo desde Swap
		if( tabla_paginas[pos_tp]->tabla[pagina-1].bit_presencia == 0 ) // page fault
			marco = pedir_pagina_swap(fd, pid, pagina);
		else // tp hit
			marco = tabla_paginas[pos_tp]->tabla[pagina-1].marco;
	}

	return marco;
}

int cargar_pagina(int pid, int pagina, void *contenido) {

	int marco = ERROR;
	int paginas_asignadas = contar_paginas_asignadas(pid);
	int p = pos_pid(pid);

	if(paginas_asignadas < config->marco_x_proceso) { // no hay necesidad de reemplazo
		marco = buscarMarcoLibre();
	} else { // hay que elegir una víctima para reemplazar
		// marco = buscarVictimaReemplazo(pid);
	}

	int pos_real = marco * config->marco_size;
	memcpy(memoria + pos_real, contenido, config->marco_size);

	subtp_t set;
	set.pagina = pagina;
	set.marco = marco;
	set.bit_presencia = 1;
	set.bit_modificado = 0;
	set.bit_uso = 1;
	setear_entrada(p, pagina-1, set);

	return marco;
}

void actualizar_tp(int pid, int pagina, int marco, int b_presencia, int b_modificacion, int b_uso) {

	int p = pos_pid(pid);

	if(marco != -1)
		tabla_paginas[p]->tabla[pagina-1].marco = marco;

	if(marco != -1)
		tabla_paginas[p]->tabla[pagina-1].bit_presencia = b_presencia;

	if(marco != -1)
		tabla_paginas[p]->tabla[pagina-1].bit_modificado = b_modificacion;

	if(marco != -1)
		tabla_paginas[p]->tabla[pagina-1].bit_uso = b_uso;

}

// </TABLA_PAGINA>


// <TLB_FUNCS>

int buscar_tlb(int pid, int pagina) {

	int pos = buscar_pos(pid, pagina);

	if(pos == ERROR) { // no encontró
		return ERROR;
	}

	tlb_t aux = tlb[pos];
	correrParaArriba(pos);
	correrParaAbajo(config->entradas_tlb-1);
	tlb[0] = aux;

	return tlb[pos].marco;
}

void agregar_tlb(int pid, int pagina, int marco) {
	correrParaAbajo(config->entradas_tlb-1);
	tlb[0].pid = pid;
	tlb[0].pagina = pagina;
	tlb[0].marco = marco;
}

void borrar_tlb(int pid, int pagina) {
	int pos = buscar_pos(pid, pagina);
	correrParaArriba(pos);
}

void correrParaArriba(int pos) {
	int i = pos;
	for(; i < config->entradas_tlb-1; i++)
		tlb[i] = tlb[i+1];
}

void correrParaAbajo(int pos) {
	int i = pos;
	for(; i > 0; i--)
		tlb[i] = tlb[i-1];
}

int buscar_pos(int pid, int pagina) {
	int pos = 0;
	while(pos < config->entradas_tlb) {
		if((tlb[pos].pid == pid) && (tlb[pos].pagina == pagina)) // encontró
			return pos;
		pos++;
	}
	return ERROR;
}

// </TLB_FUNCS>


// <AUXILIARES>

void iniciarEstructuras() {

	// semáforos
	sem_init(&mutex, 0, 1);

	// memoria
	int sizeof_memoria = config->marcos * config->marco_size;
	memoria = reservarMemoria(sizeof_memoria); // Google Chrome be like
	memset(memoria, '\0', sizeof_memoria);

	// bitmap
	bitmap = (int*)reservarMemoria(INT * config->marcos);
	int i = 0;
	for(; i < config->marcos; i++)
		bitmap[i] = 0;

	// tabla de página
	iniciarTP();

	// tlb
	int sizeof_tlb = config->entradas_tlb * sizeof(tlb_t);
	tlb = reservarMemoria(sizeof_tlb);
	int k = 0;
	for(; k < config->entradas_tlb; k++)
		tlb[k].pid = -1;

	// pids (array de pid activo por CPU)
	int j = 0;
	for(; j < MAX_CONEXIONES; j++) {
		pids[j].fd = -1;
		pids[j].pid = -1;
	}

}

void liberarConfig() {
	free(config->ip_swap);
	free(config);
}

void compararProtocolos(int protocolo1, int protocolo2) {
	if(protocolo1 != protocolo2) {
		fprintf(stderr, "Error: se esperaba protocolo #%d y se obtuvo protocolo #%d\n", protocolo2, protocolo1);
		exit(ERROR);
	}
}

void liberarRecusos() {
	// liberar otros recursos
	free(memoria);
	free(tlb);
	sem_destroy(&mutex);
	liberarConfig();
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

// </AUXILIARES>


// <PID_FUNCS>

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

	if(pos == ERROR) { // no encontró fd
		fprintf(stderr, "<actualizarPid> #%d no tiene un PID asignado\n", fd);
		return;
	}

	pids[pos].pid = pid;
}

void agregarPid(int fd, int pid) {

	int pos = buscarEspacioPid();
	if(pos == ERROR) {
		perror("<agregarPid> Se sobrepasó el máximo de conexiones");
	}
	pids[pos].fd = fd;
	pids[pos].pid = pid;
}

int buscarEspacioPid() {

	int i = 0;
	while(i < MAX_CONEXIONES) {
		if(pids[i].fd == -1) // encontró espacio libre
			return i;
		i++;
	}
	return ERROR;
}

void borrarPid(int fd) {

	int pos = 0;
	while(pos < MAX_CONEXIONES) {
		if(pids[pos].fd == fd) {
			pids[pos].fd = -1;
			pids[pos].pid = -1;
		}
		pos++;
	}
}

// </PID_FUNCS>


// <MEMORIA_FUNCS>

void borrarMarco(int marco) {
	int direccion = marco * config->marco_size;
	memset(memoria + direccion, '\0', config->marco_size);
}

int buscarMarcoLibre() {
	int marco = 0;
	while(marco < config->marcos) {
		if(bitmap[marco] == 0)
			return marco;
		marco++;
	}
	return ERROR;
}

// </MEMORIA_FUNCS>

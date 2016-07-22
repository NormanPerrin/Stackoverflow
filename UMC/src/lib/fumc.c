#include "fumc.h"

// Globales
int exitFlag = FALSE; // exit del programa

void setearValores_config(t_config * archivoConfig) {
	config = (t_configuracion*)reservarMemoria(sizeof(t_configuracion));
	config->ip_swap = (char*)reservarMemoria(CHAR*20);
	config->backlog = config_get_int_value(archivoConfig, "BACKLOG");
	config->puerto = config_get_int_value(archivoConfig, "PUERTO");
	config->ip_swap = strdup(config_get_string_value(archivoConfig, "IP_SWAP"));
	config->algoritmo = strdup(config_get_string_value(archivoConfig, "ALGORITMO"));
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

	printf("Hola! Ingresá \"salir\" para finalizar módulo y \"ayuda\" para ver los comandos\n");
	char *mensaje = reservarMemoria(CHAR * MAX_CONSOLA);

	while(!exitFlag) {

		scanf("%[^\n]%*c", mensaje);
		funcion_t *mensaje_separado = separarMensaje(mensaje);

		void (*funcion)(char*) = direccionarConsola(mensaje_separado->cabeza); // elijo función a ejecutar según mensaje
		if(funcion == NULL) {
			printf("Error: Comando no reconocido \"%s\". Escribe \"ayuda\" para ver los comandos\n", mensaje_separado->cabeza);
			free(mensaje_separado->cabeza);
			free(mensaje_separado->argumento);
			free(mensaje_separado);
			continue;
		}
		funcion(mensaje_separado->argumento);
		free(mensaje_separado->cabeza);
		free(mensaje_separado->argumento);
		free(mensaje_separado);
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

	while(!exitFlag) {
		void *mensaje = aplicar_protocolo_recibir(sockCliente, head); // recibos mensaje
		if(mensaje == NULL) {
			free(mensaje);
			break;
		}
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

	dormir(config->retardo * 2); // escritura en memoria y en tabla páginas

	int marco = ERROR;

	// pido página a Swap
	solicitudLeerPagina pedido;
	pedido.pid = pid;
	pedido.pagina = pagina;
	printf("> [PEDIR_PAGINA_SWAP]: (#fd: %d) (#pid: %d) (#pagina: %d)\n", fd, pid, pagina);
	aplicar_protocolo_enviar(sockClienteDeSwap, LEER_PAGINA, &pedido);

	int *protocolo = (int*)reservarMemoria(INT);

	// espero respuesta válida o inválida de Swap
	int *respuesta = NULL;
	respuesta = (int*)aplicar_protocolo_recibir(sockClienteDeSwap, protocolo);
	if(*respuesta == NO_PERMITIDO || *protocolo != RESPUESTA_PEDIDO) {
		*respuesta = NO_PERMITIDO;
		aplicar_protocolo_enviar(fd, RESPUESTA_PEDIDO, respuesta);
		free(respuesta);
		return ERROR;
	}

	// espero respuesta de Swap
	void *contenido_pagina = NULL;
	if(*respuesta == PERMITIDO)
		contenido_pagina = aplicar_protocolo_recibir(sockClienteDeSwap, protocolo);

	if(*protocolo != DEVOLVER_PAGINA) { // hubo un fallo

		int* estadoDelPedido = (int*)reservarMemoria(INT);
		*estadoDelPedido= NO_PERMITIDO;
		aplicar_protocolo_enviar(fd, RESPUESTA_PEDIDO, estadoDelPedido);
		free(estadoDelPedido);

	} else // tengo que cargar la página a MP
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

	int *respuesta = NULL;

	dormir(config->retardo); // retardo por escritura en tabla páginas

	inicioPrograma *mensaje = (inicioPrograma*)msj; // casteo
	printf("> [INICIAR_PROGRAMA]: (#fd: %d) (#pid: %d) (#paginas: %d)\n", fd, mensaje->pid, mensaje->paginas);

	// 1) Agrego a tabla de páginas
	iniciar_principales(mensaje->pid, mensaje->paginas);
	agregar_paginas_nuevas(mensaje->pid, mensaje->paginas);
	if( asignarMarcos(mensaje->pid) == FALSE ) {
		*respuesta = NO_PERMITIDO;
		aplicar_protocolo_enviar(fd, RESPUESTA_PEDIDO, respuesta);
	}

	// 2) Envío a Swap
	aplicar_protocolo_enviar(sockClienteDeSwap, INICIAR_PROGRAMA, msj);

	// 3) Espero respuesta de Swap
	int *protocolo = (int*)reservarMemoria(INT);
	respuesta = (int*)aplicar_protocolo_recibir(sockClienteDeSwap, protocolo);
	compararProtocolos(*protocolo, RESPUESTA_PEDIDO); // comparo protocolo recibido con esperado
	free(protocolo);

	// 4) Respondo a Núcleo como salió la operación
	aplicar_protocolo_enviar(fd, RESPUESTA_PEDIDO, respuesta);
	free(respuesta);

}

void leer_instruccion(int fd, void *msj) {

	dormir(config->retardo); // retardo por lectura en memoria

	solicitudLectura *mensaje = (solicitudLectura*)msj; // casteo

	// veo cual es el pid activo de esta CPU
	int pos = buscarPosPid(fd);
	int pid = pids[pos].pid;
	printf("> [LEER_INSTRUCCION]: (#fd: %d) (#pid: %d) (#pagina: %d) (#offset: %d) (#tamanio: %d)\n", fd, pid, mensaje->pagina, mensaje->offset, mensaje->tamanio);

	// busco el marco de la página en TLB TP y Swap
	int marco = buscarPagina(fd, pid, mensaje->pagina);

	// actualizo TLB y TP
	actualizar_tlb(pid, mensaje->pagina); // la función buscar actualiza referencia también
	actualizar_tp(pid, mensaje->pagina, marco, 1, -1, 1);

	// busco el código que me piden
	void *contenido = reservarMemoria(mensaje->tamanio);
	int pos_real = marco * (config->marco_size) + mensaje->offset;

	// tengo en cuenta si la instrucción está cortada
	if( (mensaje->offset + mensaje->tamanio) > config->marco_size ) { // tengo que pedir otra pagina más

		// agrego primer "cachito"
			int nuevo_tamanio = config->marco_size - mensaje->offset;
			memcpy(contenido, memoria + pos_real, nuevo_tamanio);

		// pido segundo "cachito"
			// busco el marco de la página en TLB TP y Swap
			int nueva_pagina = mensaje->pagina + 1;
			marco = buscarPagina(fd, pid, nueva_pagina);
			// actualizo TLB y TP
			actualizar_tlb(pid, nueva_pagina); // la función buscar actualiza referencia también
			actualizar_tp(pid, nueva_pagina, marco, 1, -1, 1);
			// concateno segundo "cachito"
			int pos_real = marco * (config->marco_size);
			int nuevo_tamanio2 = mensaje->tamanio - nuevo_tamanio;
			memcpy(contenido + nuevo_tamanio, memoria + pos_real, nuevo_tamanio2);

	} else { // no hay problemas raros

		memcpy(contenido, memoria + pos_real, mensaje->tamanio);

	}

	// respondo que pedido fue válido
	int *respuesta = (int*)reservarMemoria(INT);
	*respuesta = PERMITIDO;
	aplicar_protocolo_enviar(fd, RESPUESTA_PEDIDO, respuesta);
	free(respuesta);

	// devuelvo el contenido solicitado
	aplicar_protocolo_enviar(fd, DEVOLVER_INSTRUCCION, contenido);

	free(contenido);

}

void leer_variable(int fd, void *msj) {

	dormir(config->retardo); // retardo por lectura en memoria

	solicitudLectura *mensaje = (solicitudLectura*)msj; // casteo

	// veo cual es el pid activo de esta CPU
	int pos = buscarPosPid(fd);
	int pid = pids[pos].pid;
	printf("> [LEER_VARIABLE]: (#fd: %d) (#pid: %d) (#pagina: %d) (#offset: %d)\n", fd, pid, mensaje->pagina, mensaje->offset);

	// busco el marco de la página en TLB TP y Swap
	int marco = buscarPagina(fd, pid, mensaje->pagina);

	// actualizo TLB y TP
	actualizar_tlb(pid, mensaje->pagina); // la función buscar actualiza referencia también
	actualizar_tp(pid, mensaje->pagina, marco, 1, -1, 1);

	// busco el código que me piden
	void *contenido = reservarMemoria(INT);
	int pos_real = marco * (config->marco_size) + mensaje->offset;
	memcpy(contenido, memoria + pos_real, INT);

	// respondo que pedido fue válido
	int *respuesta = (int*)reservarMemoria(INT);
	*respuesta = PERMITIDO;
	aplicar_protocolo_enviar(fd, RESPUESTA_PEDIDO, respuesta);
	free(respuesta);

	// devuelvo el contenido solicitado
	aplicar_protocolo_enviar(fd, DEVOLVER_VARIABLE, contenido);

	free(contenido);

}

void escribir_bytes(int fd, void *msj) {

	dormir(config->retardo); // retardo por lectura en memoria

	solicitudEscritura *mensaje = (solicitudEscritura*)msj; // casteo

	// veo cual es el pid activo de esta CPU
	int pos = buscarPosPid(fd);
	int pid = pids[pos].pid;
	printf("> [ESCRIBIR_BYTES]: (#fd: %d) (#pid: %d) (#pagina: %d) (#offset: %d) (#contenido: %d)\n", fd, pid, mensaje->pagina, mensaje->offset, mensaje->contenido);

	// busco el marco de la página en TLB TP y Swap
	int marco = buscarPagina(fd, pid, mensaje->pagina);

	if(marco != ERROR) {
		// actualizo TLB y TP
		actualizar_tlb(pid, mensaje->pagina); // la fución buscar actualiza referencia también
		actualizar_tp(pid, mensaje->pagina, marco, 1, 1, 1);

		// escribo contenido
		int pos_real = marco * (config->marco_size) + mensaje->offset;
		memcpy(memoria + pos_real, &(mensaje->contenido), INT);

		// respondo a CPU
		int *respuesta = (int*)reservarMemoria(INT);
		*respuesta = PERMITIDO;
		aplicar_protocolo_enviar(fd, RESPUESTA_PEDIDO, respuesta);
		free(respuesta);
	}

}

void finalizar_programa(int fd, void *msj) {

	dormir(config->retardo * 2); // retardo por borrar entradas en tabla páginas y memoria

	int pid = *((int*)msj);
	printf("> [FINALIZAR_PROGRAMA]: (#fd: %d) (#pid: %d)\n", fd, pid);

	// aviso a Swap
	aplicar_protocolo_enviar(sockClienteDeSwap, FINALIZAR_PROGRAMA, msj);

	// recibo respuesta de Swap
	int *protocolo = (int*)reservarMemoria(INT);
	int *respuestaSwap = NULL;
	respuestaSwap = (int*)aplicar_protocolo_recibir(sockClienteDeSwap, protocolo);
	if(*respuestaSwap == NO_PERMITIDO || *protocolo != RESPUESTA_PEDIDO) {
		aplicar_protocolo_enviar(fd, RESPUESTA_PEDIDO, respuestaSwap);
		free(respuestaSwap);
		return;
	}

	// recorro páginas de pid
	int pos = pos_pid(pid);
	if(pos == ERROR) {
		int *respuestaCPU = (int*)reservarMemoria(INT);
		*respuestaCPU = NO_PERMITIDO;
		aplicar_protocolo_enviar(fd, RESPUESTA_PEDIDO, respuestaCPU);
		free(respuestaCPU);
		return;
	}

	int i = 0;
	for(; i < tabla_paginas[pos].paginas; i++) {

		if(tabla_paginas[pos].tabla[i].bit_presencia == 1) // elimino página de MP
			borrarMarco(tabla_paginas[pos].tabla[i].marco);

		// elimino página de TP
		eliminar_pagina(pid, i);

	}

	// elimino páginas de TLB si hay
	borrar_tlb(pid);

	// elimino pid de tabla paginas
	tabla_paginas[pos].pid = -1;
	tabla_paginas[pos].paginas = MAX_PAGINAS;
	tabla_paginas[pos].puntero = 0;

	// borro marcos asignados
	int m = 0;
	for(; m < config->marco_x_proceso; m++) {
		tabla_paginas[pos].marcos_reservados[m] = -1;
	}

}

// | int pid |
void cambiarPid(int fd, void *mensaje) {
	int *pid = (int*)mensaje;
	printf("> [INDICAR_PID]: (#fd: %d) (#pid: %d)\n", fd, *pid);
	actualizarPid(fd, *pid);
}

// </PRINCIPAL>


// <TABLA_PAGINA>

void iniciarTP() {
	int i;
	for(i = 0; i < MAX_PROCESOS; i++) {
		tabla_paginas[i].pid = -1;
		tabla_paginas[i].paginas = MAX_PAGINAS;
		tabla_paginas[i].marcos_reservados = (int*)reservarMemoria(config->marco_x_proceso * INT);
		int k = 0;
		for(; k < config->marco_x_proceso; k++)
			tabla_paginas[i].marcos_reservados[k] = -1;
		reset_entrada(i);
	}
}

void reset_entrada(int pos) {

	subtp_t set;
	set.pagina = -1;
	set.marco = -1;
	set.bit_presencia = 0;
	set.bit_uso = 0;
	set.bit_modificado = 0;


	int subpos = 0;
	while(subpos < tabla_paginas[pos].paginas) {
		setear_entrada(pos, subpos, set);
		subpos++;
	}
}

void setear_entrada(int pos, int subpos, subtp_t set) {
	tabla_paginas[pos].tabla[subpos].pagina = set.pagina;
	tabla_paginas[pos].tabla[subpos].marco = set.marco;
	tabla_paginas[pos].tabla[subpos].bit_presencia = set.bit_presencia;
	tabla_paginas[pos].tabla[subpos].bit_uso = set.bit_uso;
	tabla_paginas[pos].tabla[subpos].bit_modificado = set.bit_modificado;
}

int pos_pid(int pid) {

	int pos = 0;
	while(pos < MAX_PROCESOS) {
		if(tabla_paginas[pos].pid == pid)
			return pos;
		pos++;
	}

	return ERROR;
}

void iniciar_principales(int pid, int paginas) {
	int pos = buscarEntradaLibre();
	tabla_paginas[pos].pid = pid;
	tabla_paginas[pos].paginas = paginas;
	tabla_paginas[pos].puntero = 0;
}

int buscarEntradaLibre() {

	int pos = 0;
	for(; pos < MAX_PROCESOS; pos++) {
		if(tabla_paginas[pos].pid == -1) // encontró una entrada libre
			break;
	}
	return pos;
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
	setear_entrada(p, pagina, set);
}


int contar_paginas_asignadas(int pid) {

	int paginas_asignadas = 0;
	int pos = pos_pid(pid);

	int i = 0;
	while(i < tabla_paginas[pos].paginas) {
		if(tabla_paginas[pos].tabla[i].bit_presencia == 1)
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
		setear_entrada(pos, pagina, set);
	}
}

int buscarPagina(int fd, int pid, int pagina) {

	if(validarPagina(pid, pagina) == FALSE) {
		int *respuesta = (int*)reservarMemoria(INT);
		*respuesta = NO_PERMITIDO;
		aplicar_protocolo_enviar(fd, RESPUESTA_PEDIDO, respuesta);
		free(respuesta);
		return ERROR;
	}

	int marco;

	// 1) busco en TLB
	// con TLB hit no entra al if
	registro_tlb *elem = buscar_tlb(pid, pagina);
	if(elem == NULL) marco = ERROR;
	else marco = elem->marco;

	if( marco == ERROR ) { // TLB miss

		dormir(config->retardo); // retardo de búsqueda en tabla página

		// 2) busco en TP
		int pos_tp = pos_pid(pid);
		if(pos_tp == ERROR) { // no se inicializó el proceso
			int *respuesta = (int*)reservarMemoria(INT);
			*respuesta = NO_PERMITIDO;
			aplicar_protocolo_enviar(fd, RESPUESTA_PEDIDO, respuesta);
			free(respuesta);
			return ERROR;
		}

		// veo si está en MP y si no encuentro cargo desde Swap
		if( tabla_paginas[pos_tp].tabla[pagina].bit_presencia == 0 ) // page fault
			marco = pedir_pagina_swap(fd, pid, pagina);
		else // tp hit
			marco = tabla_paginas[pos_tp].tabla[pagina].marco;
	}

	return marco;
}

int cargar_pagina(int pid, int pagina, void *contenido) {

	int marco = ERROR;
	int paginas_asignadas = contar_paginas_asignadas(pid);

	if(paginas_asignadas < config->marco_x_proceso) { // no hay necesidad de reemplazo

		marco = buscarMarcoLibre(pid);

	} else { // hay que elegir una víctima para reemplazar y actualizar tp

		subtp_t pagina_reemplazar = buscarVictimaReemplazo(pid);
		verificarEscrituraDisco(pagina_reemplazar, pid); // bit M víctima = 1. se escribe a disco
		actualizar_tp(pid, pagina, pagina_reemplazar.marco, 1, -1, 1); // le asigno el marco a la página
		actualizar_tp(pid, pagina_reemplazar.pagina, -1, 0, 0, 0); // seteo en -1 el marco de la víctima y sus bits 0
		actualizarPuntero(pid, pagina); // seteo el puntero a la próxima página
		marco = pagina_reemplazar.marco;

	}

	// escribo en memoria principal
	int pos_real = marco * config->marco_size;
	memcpy(memoria + pos_real, contenido, config->marco_size);

	// actualizo tabla paginas
	subtp_t set;
	set.pagina = pagina;
	set.marco = marco;
	set.bit_presencia = 1;
	set.bit_modificado = 0;
	set.bit_uso = 1;
	int p = pos_pid(pid);
	setear_entrada(p, pagina, set);

	// actualizo tlb si esta activada
	if(config->entradas_tlb != 0)
		agregar_tlb(pid, pagina, marco);

	return marco;
}

subtp_t buscarVictimaReemplazo(int pid) {

	// 1) seteo paginas para pasar a función aplicar_algoritmo
	int p = pos_pid(pid);
	int paginas_asignadas = contar_paginas_asignadas(pid);

	subtp_t *paginas = (subtp_t*)reservarMemoria(paginas_asignadas * sizeof(subtp_t));

	int i = 0, // va a ir hasta páginas asignadas
	pos_tp = 0; // va a ir hasta total páginas del pid

	while( pos_tp < tabla_paginas[p].paginas ) {

		if( tabla_paginas[p].tabla[pos_tp].marco != -1 ) { // seteo todas las páginas con marcos para ser evaluadas
			paginas[i].pagina = tabla_paginas[p].tabla[pos_tp].pagina;
			paginas[i].marco = tabla_paginas[p].tabla[pos_tp].marco;
			paginas[i].bit_uso = tabla_paginas[p].tabla[pos_tp].bit_uso;
			paginas[i].bit_presencia = tabla_paginas[p].tabla[pos_tp].bit_presencia;
			paginas[i].bit_modificado = tabla_paginas[p].tabla[pos_tp].bit_modificado;
			i++;
		}

		pos_tp++;
	}


	// 2) aplico algoritmo y me devuelve quién debe ser reemplazado
	subtp_t pagina_reemplazada = aplicar_algoritmo(paginas, tabla_paginas[p].puntero);

	free(paginas);
	return pagina_reemplazada;
}

void actualizar_tp(int pid, int pagina, int marco, int b_presencia, int b_modificacion, int b_uso) {

	int p = pos_pid(pid);

	if(marco != -1)
		tabla_paginas[p].tabla[pagina].marco = marco;

	if(marco != -1)
		tabla_paginas[p].tabla[pagina].bit_presencia = b_presencia;

	if(marco != -1)
		tabla_paginas[p].tabla[pagina].bit_modificado = b_modificacion;

	if(marco != -1)
		tabla_paginas[p].tabla[pagina].bit_uso = b_uso;

}

// </TABLA_PAGINA>


// <TLB_FUNCS>

int tlbListIndex(int pid, int pagina) {
	int i = 0;
	registro_tlb *aux = NULL;
	for (; i < list_size(tlb); i++){
		aux = (registro_tlb*)list_get(tlb, i);
		if( (aux->pid == pid) && (aux->pagina == pagina) ) {
			free(aux);
			return i; // el proceso está en la posición 'i'
		}
			free(aux);
	}
	return ERROR; // no se encontró el proceso
}

registro_tlb *buscar_tlb(int pid, int pagina) {

	bool esElementoTlb(registro_tlb *elemento){ return (elemento->pagina == pagina && elemento->pid == pid );}
	registro_tlb* elemento = list_find(tlb, (void*) esElementoTlb);

	return elemento;
}

int actualizar_tlb(int pid, int pagina) {

	bool esElementoTlb(registro_tlb *elemento){ return (elemento->pagina == pagina && elemento->pid == pid );}
	registro_tlb* elem_removido = list_remove_by_condition(tlb, (void*) esElementoTlb);
	if(elem_removido == NULL){
		return ERROR;
	}
	else{
		list_add_in_index(tlb, 0, elem_removido);
		return TRUE;
	}
}

void agregar_tlb(int pid, int pagina, int marco) {
	if(list_size(tlb) == config->entradas_tlb){
		free( list_remove(tlb, config->entradas_tlb - 1) );
	}
	registro_tlb *elem = (registro_tlb*)reservarMemoria(sizeof(registro_tlb));
	elem->pid = pid;
	elem->pagina = pagina;
	elem->marco = marco;
	list_add_in_index(tlb, 0, elem);
}

void borrar_tlb(int pid) {
	registro_tlb *elem = NULL;
	bool esElementoTlb(registro_tlb *elemento){return elemento->pid == pid;}
	while( (elem = list_remove_by_condition(tlb, (void*) esElementoTlb)) != NULL) {
		free(elem);
	}
}

// </TLB_FUNCS>


// <AUXILIARES>

int validarPagina(int pid, int pagina) {
	int pos = pos_pid(pid);
	int paginas = tabla_paginas[pos].paginas;
	if( (pagina >= paginas) || (pagina < 0) ) return FALSE;
	return TRUE;
}

void iniciarEstructuras() {

	// logger
	logger = log_create("UMC_LOG.log", "UMC_LOG.log", TRUE, LOG_LEVEL_INFO);

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
	tlb = list_create();

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
	list_destroy(tlb);
	free(bitmap);
	sem_destroy(&mutex);
	liberarConfig();
	log_destroy(logger);
}

void *elegirFuncion(protocolo head) {

	switch(head) {

		case INICIAR_PROGRAMA:
			return inciar_programa;

		case PEDIDO_LECTURA_INSTRUCCION:
			return leer_instruccion;

		case PEDIDO_LECTURA_VARIABLE:
			return leer_variable;

		case PEDIDO_ESCRITURA:
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

void *direccionarConsola(char *mensaje) {

	if(!strcmp(mensaje, "retardo"))
		return retardo;

	if(!strcmp(mensaje, "dump"))
		return dump;

	if(!strcmp(mensaje, "flush"))
		return flush;

	if(!strcmp(mensaje, "salir")) {
		exitFlag = TRUE;
		return salir;
	}

	if(!strcmp(mensaje, "ayuda"))
		return help;

	return NULL;
}

funcion_t *separarMensaje(char *mensaje) {
	char *cabeza = (char*)reservarMemoria(CHAR*10);
	char *argumento = (char*)reservarMemoria(CHAR*10);
	sscanf(mensaje, "%s %s\n", cabeza, argumento);
	funcion_t *retorno = (funcion_t*)reservarMemoria(sizeof(funcion_t));
	retorno->cabeza = cabeza;
	retorno->argumento = argumento;
	return retorno;
}

subtp_t aplicar_algoritmo(subtp_t *paginas, int puntero) {

	subtp_t pagina_reemplazar;

	if( !strcmp(config->algoritmo, "CLOCK") )
		pagina_reemplazar  = aplicarClock(paginas, puntero);

	if( !strcmp(config->algoritmo, "CLOCK-M") )
		pagina_reemplazar  = aplicarClockM(paginas, puntero);

	return pagina_reemplazar;
}

void verificarEscrituraDisco(subtp_t pagina_reemplazar, int pid) {

	if(pagina_reemplazar.bit_modificado == 1) { // tengo que escribir en disco

		// seteo pedido
		solicitudEscribirPagina *pedido = NULL;
		pedido->pid = pid;
		pedido->pagina = pagina_reemplazar.pagina;
		void *dir_real = memoria + pagina_reemplazar.marco * config->marco_size;
		memcpy(pedido->contenido, dir_real, config->marco_size);

		// envío pedido
		aplicar_protocolo_enviar(sockClienteDeSwap, ESCRIBIR_PAGINA, pedido);

	} // sino se puede reemplazar tranquilamente

}

char *generarStringInforme(int pid, int paginas, int puntero, subtp_t *tabla) {

	char *mensaje = (char*)reservarMemoria(CHAR * 10000);
	char *linea = (char*)reservarMemoria(CHAR * 1000);
	mensaje[0] = '\0';

	sprintf(linea, "> #PID: %d;\t#Paginas: %d;\t#Puntero: %d;\n", pid, paginas, puntero);
	strcat(mensaje, linea);
	sprintf(linea, "#Pagina\t#Marco\t#Presencia\t#Uso\t#Modificado\n");
	strcat(mensaje, linea);

	int i = 0;
	for(; i < paginas; i++) {
		sprintf(linea, "\t%d\t ", tabla[i].pagina);
		strcat(mensaje, linea);
		sprintf(linea, "\t%d\t", tabla[i].marco);
		strcat(mensaje, linea);
		sprintf(linea, "\t%d\t", tabla[i].bit_presencia);
		strcat(mensaje, linea);
		sprintf(linea, "\t%d\t", tabla[i].bit_uso);
		strcat(mensaje, linea);
		sprintf(linea, "\t%d\n", tabla[i].bit_modificado);
		strcat(mensaje, linea);
	}

	free(linea);
	return mensaje;
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
		agregarPid(fd, pid);
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
	memset(memoria + direccion, '\0', config->marco_size); // borro MP
	bitmap[marco] = 0; // actualizo bitmap
}

int buscarMarcoLibre(int pid) {
	int pos = pos_pid(pid);
	int cant_paginas_ocupadas = contar_paginas_asignadas(pid);
	if(cant_paginas_ocupadas != config->marco_x_proceso) {
		int i = 0;
		for(; i < config->marco_x_proceso; i++) {
			int marco = tabla_paginas[pos].marcos_reservados[i];
			if(verificarMarcoLibre(pid, marco))
				return marco;
		}
	}
	return ERROR;
}

int verificarMarcoLibre(int pid, int marco) {
	int pos = pos_pid(pid);
	int paginas = tabla_paginas[pos].paginas;
	int i = 0;
	for(; i < paginas; i++) {
		if(tabla_paginas[pos].tabla[i].marco == marco)
			return FALSE;
	}
	return TRUE;
}

int asignarMarcos(int pid) {

	int pos = pos_pid(pid);

	int cont_locales = 0, cont_globales = 0;
	for(; cont_locales < config->marco_x_proceso; cont_locales++) {

		for(; cont_globales < config->marcos; cont_globales++) {
			if(bitmap[cont_globales] == 0) {
				break;
			}
		}

		if(bitmap[cont_globales] != 0)
			return FALSE;

		tabla_paginas[pos].marcos_reservados[cont_locales] = cont_globales;
		bitmap[cont_globales] = 1;
	}

	return TRUE;
}

// </MEMORIA_FUNCS>


// <ALGORITMOS>

subtp_t aplicarClock(subtp_t paginas[], int puntero) {

	int cantidad_paginas = sizeof(paginas) / sizeof(subtp_t);

	// 0) Busco la posición del puntero
	int pagina_inicio = 0;
	while(pagina_inicio < cantidad_paginas) {
		if(paginas[pagina_inicio].pagina == puntero) break; // encontré página inicio
		pagina_inicio++;
	}

	// 1) Busco bit U = 0 desde página inicio hasta cantidad páginas
	int i = pagina_inicio;
	for(; i < cantidad_paginas; i++) {
		if(paginas[i].bit_uso == 0) return paginas[i]; // encontré página a reemplazar
		paginas[i].bit_uso = 0; // si no encontré, modifico bit U al ir pasando
	}

	// 2) Busco bit U = 0 desde 0 hasta página inicio
	i = 0;
	for(; i < pagina_inicio; i++) {
		if(paginas[i].bit_uso == 0) return paginas[i]; // encontré página a reemplazar
		paginas[i].bit_uso = 0; // si no encontré, modifico bit U al ir pasando
	}

	// 3) Si no encontré en la pasada entonces devuevo la página desde donde empecé
	return paginas[pagina_inicio];
}


subtp_t aplicarClockM(subtp_t paginas[], int puntero) {

	int cantidad_paginas = sizeof(paginas) / sizeof(subtp_t);

	// 0) Busco la posición del puntero
	int pagina_inicio = 0;
	while(pagina_inicio < cantidad_paginas) {
		if(paginas[pagina_inicio].pagina == puntero) break; // encontré página inicio
		pagina_inicio++;
	}

	// 1) Hago una pasada buscando 00(UM)
	int i = pagina_inicio;
	for(; i < cantidad_paginas; i++) { // desde página inicio hacia cantidad páginas
		if(paginas[i].bit_uso == 0 && paginas[i].bit_modificado == 0)
			return paginas[i]; // encontré página a reemplazar
	}
	i = 0;
	for(; i < pagina_inicio; i++) { // desde 0 hasta página inicio
		if(paginas[i].bit_uso == 0 && paginas[i].bit_modificado == 0)
			return paginas[i]; // encontré página a reemplazar
	}

	// 2) Hago una pasada buscando 01(UM)
	i = pagina_inicio;
	for(; i < cantidad_paginas; i++) { // desde página inicio hacia cantidad páginas
		if(paginas[i].bit_uso == 0 && paginas[i].bit_modificado == 1)
			return paginas[i]; // encontré página a reemplazar
		else
			paginas[i].bit_uso = 0;
	}
	i = 0;
	for(; i < pagina_inicio; i++) { // desde 0 hasta página inicio
		if(paginas[i].bit_uso == 0 && paginas[i].bit_modificado == 1)
			return paginas[i]; // encontré página a reemplazar
		else
			paginas[i].bit_uso = 0;
	}

	// si todavía no encontró entonces aplico el algorítmo de nuevo
	return( aplicarClockM(paginas, puntero) );
}

void actualizarPuntero(int pid, int pagina) {

	int pos = pos_pid(pid);
	int encontro = FALSE;

	int i = pagina+1;
	for(; i < tabla_paginas[pos].paginas; i++) {
		if(tabla_paginas[pos].tabla[i].marco != -1) { // es el siguiente a la página
			tabla_paginas[pos].puntero = i;
			encontro = TRUE;
			break;
		}
	}

	if(encontro) return; // si en el for encontró no hace falta hacer el otro for

	i = 0;
	for(; i < pagina; i++) {
		if(tabla_paginas[pos].tabla[i].marco != -1) { // es el siguiente a la página
			tabla_paginas[pos].puntero = i;
			break;
		}
	}
}

// </ALGORITMOS>


// <CONSOLA_FUNCS>

void help(char *argumento) {
	printf("\nComandos:\n");
	printf("> retardo num (en milisegundos. cambia retardo a num)\n");
	printf("> dump (genera informe tabla paginas y memoria)\n");
	printf("> flush tlb | memory (tlb: limpia contenido de tlb | memory: marca todas paginas de proceso como modificadas)\n\n");
}

void retardo(char *argumento) {
	config->retardo = atoi(argumento);
}

void dump(char *argumento) {

	char *mensaje;

	// 1) Generar reporte Tabla Paginas
	int i = 0;
	for(; i < MAX_PROCESOS; i++) {
		if(tabla_paginas[i].pid == -1) continue; // me salteo las entradas sin procesos
		mensaje = generarStringInforme(tabla_paginas[i].pid, tabla_paginas[i].paginas, tabla_paginas[i].puntero, tabla_paginas[i].tabla);
		log_info(logger, "Tabla Paginas:\n%s", mensaje);
		free(mensaje);
	}

	char *marco = (char*)reservarMemoria(CHAR * config->marco_size);
	marco[0] = '\0';

	// 2) Generar reporte Memoria Principal
	int j = 0;
	for(; j < config->marcos; j++) {
		void *posicion_mp = memoria + (j * config->marco_size);
		memcpy(marco, posicion_mp, config->marco_size);
		log_info(logger, "Contenido pagina #%d:\n%s\n", j, marco);
	}

	free(marco);
}

void flush(char *argumento) {

	if(!strcmp(argumento, "tlb")) {
		list_clean(tlb);
		printf("Se ha limpiado la tlb\n");
	}

	if(!strcmp(argumento, "memory")) {
		cambiarModificado();
		printf("Se ha cambiado las paginas de modificado\n");
	}

	if( strcmp(argumento, "tlb") && strcmp(argumento, "memory") )
		fprintf(stderr, "Error: argumento \"%s\" invalido.\n- flush tlb\n- flush memory\n", argumento);
}

void salir() {
	printf("Saliendo\n");
	liberarRecusos();
	close(sockClienteDeSwap);
	close(sockServidor);
	exit(1);
}

void cambiarModificado() {
	int i = 0;
	for(; i < MAX_PROCESOS; i++) {
		if( tabla_paginas[i].pid != -1 ) { // Hay un proceso asignado en esta posicion
			int j = 0;
			for(; j < tabla_paginas[i].paginas; i++)
				tabla_paginas[i].tabla[j].bit_modificado = 1;
		}
	}
}

// </CONSOLA_FUNCS>

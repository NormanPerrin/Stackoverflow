#include "fswap.h"


// Globales
t_configuracion *config; // guarda valores config
int sockUMC; // socket cliente UMC
t_tablaDePaginas *tablaPaginas;
t_bitMap *tablaDeBitMap;
FILE *archivoSwap;
int paginasLibresTotales;


// Funciones
void setearValores_config(t_config * archivoConfig) {
	config = (t_configuracion*)reservarMemoria(sizeof(t_configuracion));
	config->nombreSwap = (char*)reservarMemoria(CHAR*30);
	config->puerto = config_get_int_value(archivoConfig, "PUERTO_ESCUCHA");
	config->nombreSwap = strdup(config_get_string_value (archivoConfig, "NOMBRE_SWAP"));
	config->cantidadPaginas = config_get_int_value(archivoConfig, "CANTIDAD_PAGINAS");
	config->tamanioPagina = config_get_int_value(archivoConfig, "TAMANIO_PAGINA");
	config->retardoCompactacion = config_get_int_value(archivoConfig, "RETARDO_COMPACTACION");
}


void escucharUMC() {

	int sockServidor;

	sockServidor = nuevoSocket();
	asociarSocket(sockServidor, config->puerto);
	escucharSocket(sockServidor, 1);

	int ret_handshake = 0;
	while(ret_handshake == 0) { // Mientras que el cliente adecuado no se conecte
		sockUMC = aceptarConexionSocket(sockServidor);
		if ( validar_conexion(sockUMC, 0) == FALSE ) continue;
		else ret_handshake = handshake_servidor(sockUMC, "S");
	}

	int *head = (int*)reservarMemoria(INT);
	while(TRUE) {
		void *mensaje = aplicar_protocolo_recibir(sockUMC, head);
		if(mensaje == NULL) {
			close(sockUMC);
			close(sockServidor);
			return;
		}
		void (*funcion)(void*) = elegirFuncion(*head); // elijo función a ejecutar según protocolo
		if(funcion == NULL) continue;
		funcion(mensaje); // ejecuto función
		funcion = NULL;
	}

	close(sockUMC);
	close(sockServidor);
}


void liberarEstructura() {
	free(config->nombreSwap);
	free(config);
}

void liberarRecusos() {
	// liberar otros recursos
	liberarEstructura();
}

int validar_cliente(char *id) {
	if(!strcmp(id, "U")) {
		printf("Cliente aceptado\n");
		return TRUE;
	} else {
		printf("Cliente rechazado\n");
		return FALSE;
	}
}
int validar_servidor(char *id) {return 0;}



FILE *inicializarSwap() {

	int tamanioSwap = config->cantidadPaginas * config->tamanioPagina;
	paginasLibresTotales = config->cantidadPaginas;

	inicializarTablaDePaginas();
	inicializarTablaBitMap();

	archivoSwap = fopen(config->nombreSwap , "w+");
	if(archivoSwap != NULL) {
		char *c = (char*)reservarMemoria(tamanioSwap);
		int l = 0;
		for(; l < tamanioSwap; l++) c[l] = '\0';
		fwrite(c, CHAR, tamanioSwap, archivoSwap);
		free(c);
	}

	return archivoSwap;
}

void inicializarTablaDePaginas() {
	tablaPaginas = (t_tablaDePaginas*)reservarMemoria(sizeof(t_tablaDePaginas) * config->cantidadPaginas);
	int i = 0;
	for (; i < config->cantidadPaginas; i++) {
		tablaPaginas[i].pid=-1;
		tablaPaginas[i].pagina=-1;
	}
}

void inicializarTablaBitMap() {
	tablaDeBitMap = (t_bitMap*)reservarMemoria(sizeof(t_bitMap) * config->cantidadPaginas);
	int i = 0;
	for(; i < config->cantidadPaginas; i++)
		tablaDeBitMap[i].ocupada = 0;
}

void iniciar_programa(void *msj) {

	int *respuesta = (int*)reservarMemoria(INT);
	*respuesta = PERMITIDO;

	inicioPrograma *mensaje = (inicioPrograma*)msj;
	int pid = mensaje->pid;
	int paginas = mensaje->paginas;

	if(paginasLibresTotales >= paginas) { // se puede alojar el proceso aunque sea compactando

		int posLibre = buscarPosLibresEnBitMap(paginas);
		if(posLibre != ERROR) { // se encontraron espacios contiguos para alojar proceso

			paginasLibresTotales -= paginas;
			int pagina = 0;
			for(; pagina < paginas ; pagina++) {
				tablaPaginas[posLibre].pid = pid;
				tablaPaginas[posLibre].pagina = pagina;
				tablaDeBitMap[posLibre].ocupada = 1;
				posLibre++;
			}

		} else { // no se encontraron espacios contiguos para alojar proceso

			dormir(config->retardoCompactacion);
			compactar();

			actualizarBitMap();

			int posLibre = buscarPosLibresEnBitMap(paginas);
			int pagina = 0;
			for(; pagina < paginas; pagina++) {
				tablaPaginas[posLibre].pid= pid;
				tablaPaginas[posLibre].pagina = posLibre;
				tablaDeBitMap[posLibre].ocupada=1;
				posLibre++;
			}

			paginasLibresTotales -= paginas;
		}

	} else *respuesta = NO_PERMITIDO; // no hay paginas disponibles para satisfacer la demanda

	if(*respuesta == PERMITIDO) {
		int pos = buscarPaginaEnTablaDePaginas(pid, 0);
		avanzarPaginas(pos);
		int tamanio = strlen(mensaje->contenido);
		fwrite(mensaje->contenido, CHAR, tamanio, archivoSwap);
	}

	aplicar_protocolo_enviar(sockUMC, RESPUESTA_PEDIDO, respuesta);
	free(respuesta);
}


void escribir_pagina(void *msj) {

	int *respuesta = (int*)reservarMemoria(INT);
	*respuesta = PERMITIDO;

	solicitudEscribirPagina *mensaje = (solicitudEscribirPagina*)msj;
	int pid = mensaje->pid;
	int pagina = mensaje->pagina;
	char *contenido = mensaje->contenido;

	int pos = buscarPaginaEnTablaDePaginas(pid, pagina);
	if(pos != ERROR) {
		avanzarPaginas(pos);
		int tamanio = strlen(contenido);
		fwrite(mensaje->contenido, CHAR, tamanio, archivoSwap);
	}
	else *respuesta = NO_PERMITIDO; // no encontro pagina en tabla de paginas

	aplicar_protocolo_enviar(sockUMC, RESPUESTA_PEDIDO, respuesta);
	free(respuesta);
}

int buscarPaginaEnTablaDePaginas(int pid, int pagina) {

	int i = 0, encontro = FALSE;
	for(; i < config->cantidadPaginas; i++) {
		if( (tablaPaginas[i].pagina == pagina) && (tablaPaginas[i].pid == pid)) {
			encontro = TRUE;
			break;
		}
	}

	if(encontro) return i;
	return ERROR;
}

void avanzarPaginas(int cantidad) {
	int avanceTotal = cantidad * config->tamanioPagina;
	fseek(archivoSwap, avanceTotal, SEEK_SET);
}

int buscarPosLibresEnBitMap(int paginas) {

	int encontro = FALSE;
	int i = 0;
	int j = 0;
	for(; i < config->cantidadPaginas; i++) {

		if(tablaDeBitMap[i].ocupada == 0) {
			while(j < paginas) {
				if(tablaDeBitMap[i+j].ocupada == 0) {
					if( (i+j) == config->cantidadPaginas ) {
						i += j;
						break;
					}
					j++;
				} else {
					i += j;
					j = 0;
					break;
				}
			} //  (while) j < paginas
		} // (if) bitmap[i] == 0

		if(j == paginas) {
			encontro = TRUE;
			break;
		}

	} // (for) i < cantidad paginas

	if(encontro) return i;
	return ERROR;
}

int buscarPosLibreEnBitMap() {

	int i = 0;
	while(i < config->cantidadPaginas) {
		if(tablaDeBitMap[i].ocupada == 0) return i;
		else i++;
	}

	return ERROR;
}

int buscarPosOcupadaDesdeLaUltimaLibreEnTablaDeBitMap(int posLibre) {

	int i = posLibre;
	for(; i < config->cantidadPaginas; i++)
		if(tablaDeBitMap[i].ocupada == 1) break;

	if(i == config->cantidadPaginas) return ERROR;

	return i;
}

int cuantasPaginasTieneElProceso(arrancaProceso) {

	int pidDelProceso = tablaPaginas[arrancaProceso].pid;
	int i = 0;

	for(; i + arrancaProceso < config->cantidadPaginas; i++)
		if(tablaPaginas[i + arrancaProceso].pid != pidDelProceso) break;

	return(i);
}



/* Funcionamiento:
 * copia la pagina de arrancaProceso a posLibre
 * posLibre++, arrancaProceso++
 * repetir cantidadDePaginasDelProceso
 * */
void mover(int posLibre, int arrancaProceso, int cantidadDePaginasDelProceso) {

	char *contenido = (char*)reservarMemoria(config->tamanioPagina);

	int i = 0;
	for(; i < cantidadDePaginasDelProceso ; i++) {

    	int pid = tablaPaginas[arrancaProceso].pid;
    	int pagina = tablaPaginas[arrancaProceso].pagina;

    	int pagABuscar = buscarPaginaEnTablaDePaginas(pid , pagina); // busco posicion para leer pagina
    	if(pagABuscar != ERROR) avanzarPaginas(pagABuscar); // avanzo puntero a inicio de pagina
    	fread(contenido, CHAR, config->tamanioPagina, archivoSwap);

    	// actualizo tablaPaginas
    	tablaPaginas[posLibre].pid = pid;
    	tablaPaginas[posLibre].pagina = pagina;
    	tablaPaginas[arrancaProceso].pid= -1;
    	tablaPaginas[arrancaProceso].pagina = -1;

    	// actualizo bitmap
    	tablaDeBitMap[posLibre].ocupada = 1;
    	tablaDeBitMap[arrancaProceso].ocupada = 0;

    	// escribo pagina de proceso en pagina libre
    	avanzarPaginas(posLibre);
    	fwrite(contenido, CHAR, config->tamanioPagina, archivoSwap);


    	// escribo \0 en la pagina arrancaProceso
		char *c = (char*)reservarMemoria(config->tamanioPagina);
		int l = 0;
		for(; l < config->tamanioPagina; l++) c[l] = '\0';
		avanzarPaginas(arrancaProceso);
		fwrite(c, CHAR, config->tamanioPagina, archivoSwap);
		free(c);

    	//actualizo contadores
    	posLibre++;
    	arrancaProceso++;
	}

	free(contenido);
}



void eliminar_programa(void *msj) {

	int *respuesta = (int*)reservarMemoria(INT);
	*respuesta = PERMITIDO;

	// casteo
	int pid = *((int*)msj);

	// busco primer aparicion del pid en tp
	int aPartirDe = buscarAPartirDeEnTablaDePaginas(pid);
	if(aPartirDe == ERROR) {
		*respuesta = NO_PERMITIDO; // no encontro pagina
	} else {

		// cuento total de paginas
	    int totalPaginas = 0;
	    int posicion = aPartirDe;
		while(tablaPaginas[posicion].pid == pid) {
			totalPaginas++;
			posicion++;
		}

		// repito como cantidad de paginas tenga el proceso
		int i = 0;
		for(; i < totalPaginas; i++) {

			// posiciono puntero en inicio de pagina
			int pagina = i + aPartirDe;
			avanzarPaginas(pagina);

			// actualizo tabla paginas
			tablaPaginas[pagina].pid = -1;

			// actualizo bitmap
			tablaDeBitMap[pagina].ocupada = 0;

			// escribo '\0' en paginas del proceso
			char *c = (char*)reservarMemoria(config->tamanioPagina);
			int l = 0;
			for(; l < config->tamanioPagina; l++) c[l] = '\0';
			fwrite(c, CHAR, config->tamanioPagina, archivoSwap);
		}

		// acutalizo contador global
		paginasLibresTotales += totalPaginas;
	}

	aplicar_protocolo_enviar(sockUMC, RESPUESTA_PEDIDO, respuesta);
	free(respuesta);
}

void leer_pagina(void *msj) {

	int *respuesta = (int*)reservarMemoria(INT);
	*respuesta = PERMITIDO;

	char *contenido = (char*)reservarMemoria(config->tamanioPagina);

	solicitudLeerPagina *mensaje = (solicitudLeerPagina*)msj;
	int pid= mensaje->pid;
	int pagina= mensaje->pagina;

	int pagABuscar = buscarPaginaEnTablaDePaginas(pid , pagina);
	if(pagABuscar != ERROR) { // encontro pagina del proceso
		avanzarPaginas(pagABuscar);
		fread(contenido, CHAR, config->tamanioPagina, archivoSwap);
	} else *respuesta = NO_PERMITIDO; // no encontro pagina solicitada del pid

	aplicar_protocolo_enviar(sockUMC, RESPUESTA_PEDIDO, respuesta);
	free(respuesta);

	if(*respuesta == PERMITIDO)
		aplicar_protocolo_enviar(sockUMC, DEVOLVER_PAGINA, contenido);

	free(contenido);
}

int buscarAPartirDeEnTablaDePaginas(int pid) {

	int i = 0, encontro = FALSE ;
	for(; i < config->cantidadPaginas; i++) {
		if(tablaPaginas[i].pid == pid) {
			encontro = TRUE;
			break;
		}
	}

	if(encontro) return i;
	return ERROR;
}


int hayFragmentacion() {

	int hayLibre = FALSE;

	int i = 0;
	for(; i < config->cantidadPaginas; i++) {

		// si encuentra una posicion libre setea el flag hayLibre en TRUE
		if(tablaDeBitMap[i].ocupada == 0)
			hayLibre = TRUE;

		// si encontro una posicion libre antes significa que hay huecos
		if(tablaDeBitMap[i].ocupada == 1)
			if(hayLibre) return TRUE;
	}

	return FALSE;
}


int compactar() {

	while(hayFragmentacion()) {

		// 1) busco primer posicion libre en bitmap
		int posLibre = buscarPosLibreEnBitMap();
		if(posLibre == ERROR) return ERROR; // no hay pagina libre

		// 2) busco primer posicion ocupada desde la libre en bitmap
		int arrancaProceso = buscarPosOcupadaDesdeLaUltimaLibreEnTablaDeBitMap(posLibre);
		if(arrancaProceso == ERROR) return ERROR; // no hay pagina ocupada desde la libre

		// 3) cuento la cantidad de paginas del proceso del bitmap ocupado
		int cantidadDePaginasDelProceso = cuantasPaginasTieneElProceso(arrancaProceso);

		// 4) muevo todas las paginas del proceso
	    mover(posLibre, arrancaProceso, cantidadDePaginasDelProceso);
	}

	return TRUE;
}

void actualizarBitMap() {
	int i = 0;
	for(; i < config->cantidadPaginas; i++) {
		if(tablaPaginas[i].pagina == -1)
			tablaDeBitMap[i].ocupada = 0;
		else
			tablaDeBitMap[i].ocupada = 1;
	}
}


void *elegirFuncion(int head) {

	switch(head) {

		case INICIAR_PROGRAMA:
			 return iniciar_programa;
			break;

		case LEER_PAGINA:
			 return leer_pagina;
			break;

		case ESCRIBIR_PAGINA:
			return escribir_pagina;
			break;

		case FINALIZAR_PROGRAMA:
			return eliminar_programa;
			break;

		default:
			fprintf(stderr, "No existe protocolo definido para %d\n", head);
			break;
	}

	return NULL;
}

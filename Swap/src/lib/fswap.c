#include "fswap.h"


// Globales
t_configuracion *config; // guarda valores config
int sockUMC; // socket cliente UMC
t_tablaDePaginas *tablaPaginas;
t_bitMap *tablaDeBitMap;
FILE *archivoSwap;
int paginasLibresTotales;
int fragmentacion; // TODO: ?


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
	fragmentacion= 0;

	inicializarTablaDePaginas();
	inicializarTablaBitMap();

	archivoSwap = fopen(config->nombreSwap , "w");
	if(archivoSwap != NULL) {
		int i = 0;
		for (;i < tamanioSwap; i++)
			fputc('\0', archivoSwap);
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

	t_tablaDePaginas *mensaje = (t_tablaDePaginas*)msj;
	int pid = mensaje->pid;
	int paginas = mensaje->pagina;

	fragmentacion = calcularFragmentacion();

	if(paginasLibresTotales > paginas) { // se puede alojar el proceso aunque sea compactando

		int posLibre = buscarPosLibresEnBitMap(paginas);
		if(posLibre != ERROR) { // se encontraron espacios contiguos para alojar proceso

			paginasLibresTotales -= paginas;
			for(; posLibre < paginas ; posLibre++) {
				tablaPaginas[posLibre].pid = pid;
				tablaPaginas[posLibre].pagina = posLibre;
				tablaDeBitMap[posLibre].ocupada = 1;
			}

		} else if(fragmentacion > paginas) { // no se encontraron espacios contiguos para alojar proceso TODO: ver

			dormir(config->retardoCompactacion);
			compactar();

			actualizarBitMap();

			int posLibre = buscarPosLibresEnBitMap(paginas);
			if(posLibre != ERROR) { // despues de compactar se encontraron espacios contiguos para proceso

				for(; posLibre<paginas ;posLibre++) {
					tablaPaginas[posLibre].pid= pid;
					tablaPaginas[posLibre].pagina = posLibre;
					tablaDeBitMap[posLibre].ocupada=1;
				}

				paginasLibresTotales -= paginas;

			} else *respuesta = NO_PERMITIDO; // despues de compactar sigue sin poder alojarse proceso

		} else *respuesta = NO_PERMITIDO; // no hay paginas disponibles para demanda

	} else *respuesta = NO_PERMITIDO; // si cant paginas totales disponibles es menor a demandadas

	aplicar_protocolo_enviar(sockUMC, RESPUESTA_PEDIDO, respuesta);
	free(respuesta);
}


void escribir_pagina(void *msj) {

	int *respuesta = (int*)reservarMemoria(INT);
	*respuesta = PERMITIDO;

	t_escribirPagina *mensaje = (t_escribirPagina*)msj;
	int pid = mensaje->pid;
	int pagina = mensaje->pagina;
	void *contenido = mensaje->contenido;

	int pos = buscarPaginaEnTablaDePaginas(pid, pagina);
	if(pos != ERROR) {
		avanzarPaginas(pos);
		fprintf(archivoSwap, "%s", (char*)contenido);
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

	char *f = (char*)reservarMemoria(CHAR);
	rewind(archivoSwap);

	int avanceTotal = cantidad * config->tamanioPagina;

	int i = 0;
	while(i < avanceTotal) {
		fscanf(archivoSwap, "%c", f);
		i++;
	}

	free(f);
}

int buscarPosLibresEnBitMap(int paginas) {

	int i =0;
	int j=0;
	while(i<config->cantidadPaginas) {

		if(tablaDeBitMap[i].ocupada ==0) {
			while(j < paginas) {
				if(tablaDeBitMap[i+j].ocupada==0) j++;
				else break;
			}
		}

		if(j == paginas) return i;
		else i += j;

		i++;
	}

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

	for(; arrancaProceso < config->cantidadPaginas; arrancaProceso++)
		if(tablaPaginas[arrancaProceso].pid != pidDelProceso) break;

	return (--arrancaProceso);
}


void mover(int posLibre, int arrancaProceso, int cantidadDePaginasDelProceso) {

	int i = 0;
	for(; i < cantidadDePaginasDelProceso ; i++) {

    	t_tablaDePaginas *mensaje = NULL;

    	mensaje->pid = tablaPaginas[arrancaProceso].pid;
    	mensaje->pagina = tablaPaginas[arrancaProceso].pagina;

    	char *contenido = (char*)reservarMemoria(config->tamanioPagina);
//    	contenido = leer_pagina((void*)mensaje);

    	tablaPaginas[posLibre].pid= tablaPaginas[arrancaProceso].pid;
    	tablaPaginas[posLibre].pagina= tablaPaginas[arrancaProceso].pagina;
    	tablaPaginas[arrancaProceso].pid=-1;
    	tablaPaginas[arrancaProceso].pagina=-1;
    	tablaDeBitMap[posLibre].ocupada=1;
    	tablaDeBitMap[arrancaProceso].ocupada=0;
    	t_escribirPagina *msj;
    	msj->pid= mensaje->pid;
    	msj->pagina=mensaje->pagina;
    	msj->contenido = (char*)reservarMemoria(config->tamanioPagina);
    	strcpy(msj->contenido, contenido);
    	free(contenido);
    	escribir_pagina((void*)msj);
    	free(msj->contenido);
    	posLibre++;
    	arrancaProceso++;

	}
}



void eliminar_programa(void *msj) {

	t_tablaDePaginas *mensaje = (t_tablaDePaginas*)msj;
	int pid= mensaje->pid;
	int i;
	int aPartirDe = buscarAPartirDeEnTablaDePaginas(pid);
	int aPartirDeAux = buscarAPartirDeEnTablaDePaginas(pid);
    int totalPaginas;
	while(tablaPaginas[aPartirDe].pid==pid) {
		aPartirDe++;
	}
	totalPaginas=aPartirDe;
	char *contenido = reservarMemoria(CHAR);
	contenido[0]='\0';
	t_escribirPagina *arg;
	for(i=0;i<totalPaginas;i++) {
		arg->contenido = contenido;
		arg->pagina = i;
		arg->pid = pid;
		escribir_pagina(arg);
	}
	free(contenido);
	while(tablaPaginas[aPartirDeAux].pid==pid) {
		tablaPaginas[aPartirDeAux].pid=-1;
		tablaPaginas[aPartirDeAux].pagina=-1;
		aPartirDeAux++;
	}

   paginasLibresTotales = paginasLibresTotales+totalPaginas;
}

void leer_pagina(void *msj) {

	int *respuesta = (int*)reservarMemoria(INT);
	*respuesta = PERMITIDO;

	char *contenido = (char*)reservarMemoria(config->tamanioPagina);

	t_tablaDePaginas *mensaje = (t_tablaDePaginas*)msj;
	int pid= mensaje->pid;
	int pagina= mensaje->pagina;

	int pagABuscar = buscarPaginaEnTablaDePaginas(pid , pagina);
	if(pagABuscar != ERROR) { // encontro pagina del proceso
		avanzarPaginas(pagABuscar);
		char c;
		int i = 0;
		for(; i < config->tamanioPagina; i++) {
			c = fgetc(archivoSwap);
			contenido[i] = c;
		}
	} else *respuesta = NO_PERMITIDO; // no encontro pagina solicitada del pid

	aplicar_protocolo_enviar(sockUMC, RESPUESTA_PEDIDO, respuesta);
	free(respuesta);

	aplicar_protocolo_enviar(sockUMC, DEVOLVER_PAGINA, contenido);
	free(contenido);
}

int buscarAPartirDeEnTablaDePaginas(int pid) {
	int i =0;

		while(tablaPaginas[i].pid != pid ) {
		   i++;
			}
		return i;
}


int calcularFragmentacion() {
	int i=0;
	int pagsLibres;
	while (i< config->cantidadPaginas) {
		if(tablaDeBitMap[i].ocupada==0) {
			pagsLibres++;

	 	}
		i++;
	}
	return pagsLibres;
}



void compactar() {
	int posLibre=buscarPosLibreEnBitMap();
	int arrancaProceso =buscarPosOcupadaDesdeLaUltimaLibreEnTablaDeBitMap(posLibre);
	int cantidadDePaginasDelProceso=cuantasPaginasTieneElProceso(arrancaProceso);
    mover(posLibre , arrancaProceso , cantidadDePaginasDelProceso);





 }

void actualizarBitMap() {
	int i =0;
	while(i< config->cantidadPaginas) {
		if(tablaPaginas[i].pagina==-1) {
			tablaDeBitMap[i].ocupada=0;
		} else tablaDeBitMap[i].ocupada=1;
		i++;
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

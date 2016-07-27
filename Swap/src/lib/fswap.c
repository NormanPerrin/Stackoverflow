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
	config->retardoAcceso = config_get_int_value(archivoConfig , "RETARDO_ACCESO");
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

	int head;
	while(TRUE) {
		void *mensaje = aplicar_protocolo_recibir(sockUMC, &head);
		if(mensaje == NULL) {
			cerrarSocket(sockUMC);
			cerrarSocket(sockServidor);
			return;
		}
		void (*funcion)(void*) = elegirFuncion(head); // elijo función a ejecutar según protocolo
		if(funcion == NULL) continue;
		funcion(mensaje); // ejecuto función
		funcion = NULL;
	}

	cerrarSocket(sockUMC);
	cerrarSocket(sockServidor);
}

void liberarEstructura() {
	free(config->nombreSwap); config->nombreSwap = NULL;
	free(config); config = NULL;
}

void liberarRecusos() {
	// liberar otros recursos
	liberarEstructura();
	free(tablaPaginas); tablaPaginas = NULL;
	free(tablaDeBitMap); tablaDeBitMap = NULL;
	fclose(archivoSwap);
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

void inicializarSwap() {

	int tamanioSwap = config->cantidadPaginas * config->tamanioPagina;
	paginasLibresTotales = config->cantidadPaginas;

	inicializarTablaDePaginas();
	inicializarTablaBitMap();

	archivoSwap = fopen(config->nombreSwap , "w+");
	if(archivoSwap != NULL) {

		char caracter = '\0';
		int i;
		for(i=0; i < tamanioSwap; i++){
			fwrite(&caracter, CHAR, CHAR, archivoSwap);
		}
	}
}

void inicializarTablaDePaginas() {
	tablaPaginas = reservarMemoria(sizeof(t_tablaDePaginas) * config->cantidadPaginas);
	int i;
	for (i=0; i < config->cantidadPaginas; i++) {
		tablaPaginas[i].pid=-1;
		tablaPaginas[i].pagina=-1;
	}
}

void inicializarTablaBitMap() {
	tablaDeBitMap = reservarMemoria(sizeof(t_bitMap) * config->cantidadPaginas);
	int i;
	for(i=0; i < config->cantidadPaginas; i++){
		tablaDeBitMap[i].ocupada = 0;
	}
}

void iniciar_programa(void *msj) {

	int *respuesta = reservarMemoria(INT);
	*respuesta = PERMITIDO;

	inicioPrograma *mensaje = (inicioPrograma*) msj;
	int pid = mensaje->pid;
	int paginas = mensaje->paginas;
	printf("> [INICIAR_PROGRAMA]: (#pid: %d) (#paginas: %d)\n", pid, paginas);

	if(paginasLibresTotales >= paginas) { // se puede alojar el proceso aunque sea compactando

		int posLibre = buscarPosLibresEnBitMap(paginas);
		if(posLibre != ERROR) { // se encontraron espacios contiguos para alojar proceso

			paginasLibresTotales -= paginas;
			int pagina;
			for(pagina=0; pagina < paginas ; pagina++) {
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
			int pagina;
			for(pagina=0; pagina < paginas; pagina++) {
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
		dormir(config->retardoAcceso);
		// Escribo la totalidad del código en el archivo:
		fwrite(mensaje->contenido, CHAR, strlen(mensaje->contenido), archivoSwap);
	}

	aplicar_protocolo_enviar(sockUMC, RESPUESTA_PEDIDO, respuesta);
	free(respuesta); respuesta = NULL;
}

void escribir_pagina(void *msj){

	solicitudEscribirPagina *mensaje = (solicitudEscribirPagina*) msj;

	printf("> [ESCRIBIR_PAGINA]: (#pid: %d) (#pagina: %d)\n", mensaje->pid, mensaje->pagina);
	int numeroDePagina = buscarPaginaEnTablaDePaginas(mensaje->pid, mensaje->pagina);
	int *respuesta = reservarMemoria(INT);

	if(numeroDePagina != ERROR){
		*respuesta = PERMITIDO;
		aplicar_protocolo_enviar(sockUMC, RESPUESTA_PEDIDO, respuesta);
		avanzarPaginas(numeroDePagina); // Desplazamiento dentro del archivo
		dormir(config->retardoAcceso);
		// Escribo el contenido recibido en el archivo:
		fwrite(mensaje->contenido, CHAR, mensaje->tamanio_marco, archivoSwap);
	}
	else{
		*respuesta = NO_PERMITIDO; // no encontró página en tabla de páginas
		aplicar_protocolo_enviar(sockUMC, RESPUESTA_PEDIDO, respuesta);
	}

	free(respuesta); respuesta = NULL;
}

int buscarPaginaEnTablaDePaginas(int pid, int pagina) {

	int i, encontro = FALSE;
	for(i=0; i < config->cantidadPaginas; i++) {
		if((tablaPaginas[i].pagina == pagina) && (tablaPaginas[i].pid == pid)) {
			encontro = TRUE;
			break;
		}
	}

	if(encontro){
		return i;
	} else{
		return ERROR; }
}

void avanzarPaginas(int numeroDePagina){
	int avanceTotal = numeroDePagina * config->tamanioPagina;
	// Me desplazo desde el inicio del archivo hasta la posición solicitada:
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

	char *contenido = reservarMemoria(config->tamanioPagina);

	int i;
	for(i=0; i < cantidadDePaginasDelProceso ; i++){

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

    	// escribo página de proceso en página libre
    	avanzarPaginas(posLibre);
    	fwrite(contenido, CHAR, config->tamanioPagina, archivoSwap);

    	// escribo \0 en la página arrancaProceso
    	char caracter = '\0';
    	int i;
    	for(i=0; i < config->tamanioPagina; i++){
    		fwrite(&caracter, CHAR, CHAR, archivoSwap);
    	}

    	//actualizo contadores
    	posLibre++;
    	arrancaProceso++;
	}

	free(contenido); contenido = NULL;
}

void eliminar_programa(void *msj){

	dormir(config->retardoAcceso);

	int *respuesta = reservarMemoria(INT);
	*respuesta = PERMITIDO;

	// casteo pid
	int pid = *((int*) msj);
	printf("> [ELIMINAR_PROGRAMA]: (#pid: %d)\n", pid);

	// busco primer aparición del pid en tabla de páginas
	int aPartirDe = buscarAPartirDeEnTablaDePaginas(pid);
	if(aPartirDe == ERROR){
		*respuesta = NO_PERMITIDO; // no encontró página
	} else {

		// cuento total de páginas
	    int totalPaginas = 0;
	    int posicion = aPartirDe;
		while(tablaPaginas[posicion].pid == pid){
			totalPaginas++;
			posicion++;
		}

		// repito como cantidad de páginas tenga el proceso
		int i;
		for(i=0; i < totalPaginas; i++){

			// posiciono puntero del archivo en inicio de página
			int pagina = i + aPartirDe;
			avanzarPaginas(pagina);

			// actualizo tabla de páginas
			tablaPaginas[pagina].pid = -1;

			// actualizo bitmap
			tablaDeBitMap[pagina].ocupada = 0;

			// escribo '\0' en paginas del proceso
			char caracter = '\0';
			int i;
			for(i=0; i < config->tamanioPagina; i++){
				fwrite(&caracter, CHAR, CHAR, archivoSwap);
			}
		}
		// acutalizo contador global
		paginasLibresTotales += totalPaginas;
	}

	aplicar_protocolo_enviar(sockUMC, RESPUESTA_PEDIDO, respuesta);
	free(respuesta); respuesta = NULL;
}

void leer_pagina(void *msj) {

	solicitudLeerPagina *mensaje = (solicitudLeerPagina*) msj;
	int pid = mensaje->pid;
	int pagina = mensaje->pagina;
	printf("> [LEER_PAGINA]: (#pid: %d) (#pagina: %d)\n", pid, pagina);

	int pagABuscar = buscarPaginaEnTablaDePaginas(pid , pagina);

	int *respuesta = reservarMemoria(INT);

	if(pagABuscar != ERROR){ // Encontró página solicitada del proceso:

		*respuesta = PERMITIDO;
		aplicar_protocolo_enviar(sockUMC, RESPUESTA_PEDIDO, respuesta);

		avanzarPaginas(pagABuscar);
		dormir(config->retardoAcceso);
		// Cargo en la página a devolver el contenido leído del archivo:
		paginaSwap* pagina_swap = malloc(sizeof(paginaSwap));
		pagina_swap->tamanio_marco = config->tamanioPagina;
		pagina_swap->contenido = malloc(config->tamanioPagina);

		fread(pagina_swap->contenido, CHAR, config->tamanioPagina, archivoSwap);

		aplicar_protocolo_enviar(sockUMC, DEVOLVER_PAGINA, pagina_swap);
		free(pagina_swap->contenido); pagina_swap->contenido = NULL;
		free(pagina_swap); pagina_swap = NULL;
	}
	else{ // No encontró página solicitada del proceso:
		*respuesta = NO_PERMITIDO;
		aplicar_protocolo_enviar(sockUMC, RESPUESTA_PEDIDO, respuesta);
	}

	free(respuesta); respuesta = NULL;
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

	printf("> [COMPACTACIÓN]\n");

	while(hayFragmentacion()){

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

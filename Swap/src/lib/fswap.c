#include "fswap.h"


// Globales
t_configuracion *config; // guarda valores config
int sockUMC; // socket cliente UMC
t_tablaDePaginas * tablaPaginas;
t_bitMap * tablaDeBitMap;
FILE * archivoSwap;
int paginasLibresTotales;
int fragmentacion;


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

		if ( validar_conexion(sockUMC, 0) == FALSE )
			continue;
		else
			ret_handshake = handshake_servidor(sockUMC, "S");

	}

	int status = 1;		// Estructura que manjea el status de los recieve.
	int *head = (int*)reservarMemoria(INT);

	while(status > 0) {
		void *mensaje = aplicar_protocolo_recibir(sockUMC, head);
		void (*funcion)(void*) = elegirFuncion(*head); // elijo función a ejecutar según protocolo
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
int validar_servidor(char *id){return 0;}



FILE * inicializarSwap (){
      int cantPags = config->cantidadPaginas;
      int tamPags = config->tamanioPagina;
      int tamanioSwap = cantPags*tamPags;
      int i;
      fragmentacion= 0;
      paginasLibresTotales= config->cantidadPaginas;
      inicializarTablaDePaginas();
      inicializarTablaBitMap();
      archivoSwap = fopen(config->nombreSwap , "w");
        if(archivoSwap != NULL){
        	for (i=0;i<tamanioSwap;i++){
        fputc('\0', archivoSwap);
       }

      }
        return archivoSwap;

}

void inicializarTablaDePaginas() {
	tablaPaginas=(t_tablaDePaginas*)reservarMemoria(sizeof(t_tablaDePaginas) * config->cantidadPaginas);
	int i;
	for (i=0 ; i<config->cantidadPaginas;i++) {
		tablaPaginas[i].pid=-1;
		tablaPaginas[i].pagina=-1;
	}
}

void inicializarTablaBitMap(){
	tablaDeBitMap=(t_bitMap*)reservarMemoria(sizeof(t_bitMap) * config->cantidadPaginas);
	int i;
	for (i=0 ; i< config->cantidadPaginas;i++){
		tablaDeBitMap[i].ocupada = 0;
		}
}

int iniciar_programa(void *msj) {

	t_tablaDePaginas *mensaje = (t_tablaDePaginas*)msj;
	int pid = mensaje->pid;
	int paginas = mensaje->pagina;

	fragmentacion = calcularFragmentacion();

	if(paginasLibresTotales > paginas){
		int posLibre= buscarPosLibresEnBitMap(paginas);

		if(posLibre != -1) {
			paginasLibresTotales = paginasLibresTotales-paginas;
			for(; posLibre<paginas ;posLibre++) {
				tablaPaginas[posLibre].pid= pid;
				tablaPaginas[posLibre].pagina = posLibre;
				tablaDeBitMap[posLibre].ocupada=1;
				}
		} else {  if(fragmentacion > paginas){
			dormir(config->retardoCompactacion);
			compactar();
			actualizarBitMap();
			int posLibre= buscarPosLibresEnBitMap(paginas);

					if(posLibre != -1){
						for(; posLibre<paginas ;posLibre++){
							tablaPaginas[posLibre].pid= pid;
							tablaPaginas[posLibre].pagina = posLibre;
							tablaDeBitMap[posLibre].ocupada=1;}


					}
					paginasLibresTotales= paginasLibresTotales-paginas;
	}else { return -1;}

		}
	}
	return 1;
}


int escribir_pagina(void *msj) {
	t_escribirPagina *mensaje = (t_escribirPagina*)msj;
	 int pid=mensaje->pid;
	 int pagina = mensaje->pagina;
	 void *contenido = mensaje->contenido;
	 int pag =buscarPaginaEnTablaDePaginas(pid ,pagina);
	  if(pag !=-1){
		  avanzarPaginas(pag);
		  fprintf(archivoSwap, "%s", (char*)contenido);
		  }
	  else {return ERROR;};
	  return 1;
}

int buscarPaginaEnTablaDePaginas(int pid ,int pagina){

	int i =0;

	while((tablaPaginas[i].pid != pid) && (tablaPaginas[i].pagina != pagina)){
	   i++;
		}
	return i;
	}

void avanzarPaginas(int cantidad){
	int i = 0;
	char* f;
	f = malloc(CHAR);
	rewind(archivoSwap);
	int avanceTotal = (cantidad)*config->tamanioPagina;
	while(i<avanceTotal){
		fscanf(archivoSwap,"%c",f);
		i++;
	}
	free(f);
}

int buscarPosLibresEnBitMap(int paginas){
	int i =0;
	int j=0;
	while(i<config->cantidadPaginas){
		if(tablaDeBitMap[i].ocupada ==0){
			while(j<paginas){
				if(tablaDeBitMap[i+j].ocupada==0){
				j++;} else{ break;}
			}

		}
		if(j==paginas){ return i;} else { i=i+j;};

		i++;
	}
    return -1;
}

int buscarPosLibreEnBitMap(){

	int i= 0 ;
	while(i<config->cantidadPaginas){
		if(tablaDeBitMap[i].ocupada==0){
		 return i;
		}else { i++;}
	}
}

int buscarPosOcupadaDesdeLaUltimaLibreEnTablaDeBitMap(int posLibre){
	int i = posLibre;
	while (tablaDeBitMap[i].ocupada==0){
		i++;
	}
	return i;
}

int cuantasPaginasTieneElProceso(arrancaProceso){
	int pidDelProceso= tablaPaginas[arrancaProceso].pid;
	while(tablaPaginas[arrancaProceso].pid==pidDelProceso){
		arrancaProceso++;
	}
	int totalPaginas=arrancaProceso;
	return totalPaginas;
}

void mover(int posLibre , int arrancaProceso ,int cantidadDePaginasDelProceso){
	int i =0;
    for (i ; i<cantidadDePaginasDelProceso ; i++){
    	t_tablaDePaginas *mensaje;
    	mensaje->pid = tablaPaginas[arrancaProceso].pid;
    	mensaje->pagina = tablaPaginas[arrancaProceso].pagina;
    	char* contenido = (char*)reservarMemoria(config->tamanioPagina);
    	contenido = leer_pagina((void*)mensaje);
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



int eliminar_programa(void *msj) {
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
	while(tablaPaginas[aPartirDeAux].pid==pid){
		tablaPaginas[aPartirDeAux].pid=-1;
		tablaPaginas[aPartirDeAux].pagina=-1;
		aPartirDeAux++;
	}

   paginasLibresTotales = paginasLibresTotales+totalPaginas;
	return 1;
}

char* leer_pagina(void *msj){
	t_tablaDePaginas *mensaje = (t_tablaDePaginas*)msj;
	int pid= mensaje->pid;
	int pagina= mensaje->pagina;
	int pagABuscar = buscarPaginaEnTablaDePaginas(pid , pagina);
	if(pagABuscar !=-1){
			 avanzarPaginas(pagABuscar);
			// fread(/*completar*/);
	}
 return NULL;
}

int buscarAPartirDeEnTablaDePaginas(int pid){
	int i =0;

		while(tablaPaginas[i].pid != pid ){
		   i++;
			}
		return i;
}


int calcularFragmentacion(){
	int i=0;
	int pagsLibres;
	while (i< config->cantidadPaginas){
		if(tablaDeBitMap[i].ocupada==0){
			pagsLibres++;

	 	}
		i++;
	}
	return pagsLibres;
}



void compactar(){
	int posLibre=buscarPosLibreEnBitMap();
	int arrancaProceso =buscarPosOcupadaDesdeLaUltimaLibreEnTablaDeBitMap(posLibre);
	int cantidadDePaginasDelProceso=cuantasPaginasTieneElProceso(arrancaProceso);
    mover(posLibre , arrancaProceso , cantidadDePaginasDelProceso);





 }

void actualizarBitMap(){
	int i =0;
	while(i< config->cantidadPaginas){
		if(tablaPaginas[i].pagina==-1){
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

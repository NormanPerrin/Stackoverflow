#include "fswap.h"

// Globales
t_configuracion *config; // guarda valores config
int sockUMC; // socket cliente UMC
t_tablaDePaginas * tablaPaginas;
t_bitMap * tablaDeBitMap;
FILE * archivoSwap;
int paginasLibresTotales;


// Funciones
void setearValores_config(t_config * archivoConfig){
	config = (t_configuracion*)reservarMemoria(sizeof(t_configuracion));
	config->nombreSwap = (char*)reservarMemoria(CHAR*30);
	config->puerto = config_get_int_value(archivoConfig, "PUERTO_ESCUCHA");
	config->nombreSwap = strdup(config_get_string_value (archivoConfig, "NOMBRE_SWAP"));
	config->cantidadPaginas = config_get_int_value(archivoConfig, "CANTIDAD_PAGINAS");
	config->tamanioPagina = config_get_int_value(archivoConfig, "TAMANIO_PAGINA");
	config->retardoCompactacion = config_get_int_value(archivoConfig, "RETARDO_COMPACTACION");
}


void escucharUMC(){

	int sockServidor;

	sockServidor = nuevoSocket();
	asociarSocket(sockServidor, config->puerto);
	escucharSocket(sockServidor, 1);

	int ret_handshake = 0;
	while(ret_handshake == 0) { // Mientras que el cliente adecuado no se conecte

		sockUMC = aceptarConexionSocket(sockServidor);

		if ( validar_conexion(sockUMC, 0) == FALSE ) {
			continue;
		} else {
			ret_handshake = handshake_servidor(sockUMC, "S");
		}

	}

	int status = 1;		// Estructura que manjea el status de los recieve.
	int head;

	while (status > 0){
		status = recibirPorSocket(sockUMC, &head, 1);
		validar_recive(status, 1); // es terminante ya que si hay un error en el recive o desconexión debe terminar
		aplicar_protocolo_recibir(sockUMC, &head, SIZE_MSG);
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
	for (i=0 ; i<config->cantidadPaginas;i++){
		tablaPaginas[i].pid=-1;
		tablaPaginas[i].marco=-1;
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

int inciar_programa(int pid,int paginas){

	if(paginasLibresTotales > paginas){
		int posLibre= buscarPosLibresEnBitMap(paginas);

		if(posLibre != -1){
			for(; posLibre<paginas ;posLibre++){
				tablaPaginas[posLibre].pid= pid;
				tablaPaginas[posLibre].pagina = posLibre;
			}
		} else{ return -1;}
	}else { return -1;}
return 1;
};


int escribir_pagina(int pid ,int pagina , void * contenido){
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

int eleminar_programa(int pid){  //falta en terminarla
	int aPartirDe = buscarAPartirDeEnTablaDePaginas(pid);

	while(tablaPaginas[aPartirDe].pid==pid){
		tablaPaginas[aPartirDe].pid=-1;
		tablaPaginas[aPartirDe].pagina=-1;
	}
	return 1;//borrrar en archivoSwap
}

int leer_pagina(int pid , int pagina){
	int pagABuscar = buscarPaginaEnTablaDePaginas(pid , pagina);
	if(pagABuscar !=-1){
			 avanzarPaginas(pagABuscar);
			// fread(/*completar*/);
	}
 return 1;
}

int buscarAPartirDeEnTablaDePaginas(int pid){
	int i =0;

		while(tablaPaginas[i].pid != pid ){
		   i++;
			}
		return i;
}

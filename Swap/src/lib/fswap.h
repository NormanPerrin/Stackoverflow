#ifndef FSWAP_H_
#define FSWAP_H_

	#include <utilidades/sockets.h>
	#include <utilidades/general.h>
	#include <commons/config.h>
	#include <commons/log.h>
    #include <utilidades/comunicaciones.h>

	#define PACKAGESIZE 1024 // Size máximo de paquete para sockets
	#define RUTA_CONFIG_SWAP "configSwap.txt"

	// Estructuras
	typedef struct {
		int puerto;
		char *nombreSwap;
		int cantidadPaginas;
		int tamanioPagina;
		int retardoCompactacion;
	} t_configuracion;

	typedef struct {
		int pagina;
		int pid;
		int marco;

	} t_tablaDePaginas;

	typedef struct{
		int ocupada;
	} t_bitMap;

	typedef struct {
		int pid;
		int pagina;
	} pedidoPagina_t;

	// Cabeceras
	void setearValores_config(t_config * archivoConfig);
	void escucharUMC();
	void liberarEstructura(); // Libera memoria asignada en setear config
	void liberarRecusos();
	int validar_cliente(char *id); // Verifica que sea cliente UMC
	int validar_servidor(char *id); // Para que no joda con error
	FILE * inicializarSwap (); //inicializa particion swap y listas
	int inciar_programa(int pid, int paginas);
	void inicializarTablaDePaginas();
	void inicializarTablaBitMap();
	int inciar_programa(int pid,int paginas);
	int buscarPosLibresEnBitMap(int paginas);
	int escribir_pagina(int pid , int pagina ,void* contenido);
	int buscarPaginaEnTablaDePaginas(int pid ,int pagina);
	void avanzarPaginas(int cantidad);
	int eleminar_programa(int pid);
	int buscarAPartirDeEnTablaDePaginas(int pid);

#endif /* FSWAP_H_ */

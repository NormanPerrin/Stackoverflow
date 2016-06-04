#ifndef FSWAP_H_
#define FSWAP_H_

	#include <utilidades/sockets.h>
	#include <utilidades/general.h>
	#include <commons/config.h>
	#include <commons/log.h>
    #include <utilidades/comunicaciones.h>

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
	} t_tablaDePaginas;

	typedef struct{
		int ocupada;
	} t_bitMap;


	// Cabeceras
	void setearValores_config(t_config * archivoConfig);
	void escucharUMC();
	void liberarEstructura(); // Libera memoria asignada en setear config
	void liberarRecusos();
	int validar_cliente(char *id); // Verifica que sea cliente UMC
	int validar_servidor(char *id); // Para que no joda con error
	FILE * inicializarSwap (); //inicializa particion swap y listas
	int inciar_programa(void *msj);
	void inicializarTablaDePaginas();
	void inicializarTablaBitMap();
	int buscarPosLibresEnBitMap(int paginas);
	int escribir_pagina(int pid , int pagina ,void* contenido);
	int buscarPaginaEnTablaDePaginas(int pid ,int pagina);
	void avanzarPaginas(int cantidad);
	int eleminar_programa(int pid);
	int buscarAPartirDeEnTablaDePaginas(int pid);
	int calcularFragmentacion();
	void compactar();
	void actualizarBitMap();

#endif /* FSWAP_H_ */

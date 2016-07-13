#ifndef FSWAP_H_
#define FSWAP_H_

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

	typedef struct{
		char *contenido;
		int pid;
		int pagina;
	}t_escribirPagina;

	// Cabeceras
	void setearValores_config(t_config * archivoConfig);
	void escucharUMC();
	void liberarEstructura(); // Libera memoria asignada en setear config
	void liberarRecusos();
	int validar_cliente(char *id); // Verifica que sea cliente UMC
	int validar_servidor(char *id); // Para que no joda con error
	FILE * inicializarSwap (); //inicializa particion swap y listas
	void iniciar_programa(void *msj);
	void inicializarTablaDePaginas();
	void inicializarTablaBitMap();
	int buscarPosLibresEnBitMap(int paginas);
	void leer_pagina(void *msj);
	int buscarPosLibreEnBitMap();
	void escribir_pagina(void *msj);
	int buscarPaginaEnTablaDePaginas(int pid ,int pagina);
	void avanzarPaginas(int cantidad);
	void eliminar_programa(void *msj);
	void *elegirFuncion(int head);
	int buscarAPartirDeEnTablaDePaginas(int pid);
	int hayFragmentacion();
	int compactar();
	void actualizarBitMap();
	int buscarPosOcupadaDesdeLaUltimaLibreEnTablaDeBitMap(int posLibre);
	int cuantasPaginasTieneElProceso(int arrancaProceso);
	void mover(int posLibre ,int arrancaProceso , int cantidadDePaginasDelProceso);


#endif /* FSWAP_H_ */

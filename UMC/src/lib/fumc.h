#ifndef LIB_FUMC_H_
#define LIB_FUMC_H_

	#include <utilidades/sockets.h>
	#include <utilidades/general.h>
	#include <utilidades/comunicaciones.h>
	#include <commons/config.h>
	#include <commons/log.h>
	#include <pthread.h>
	#include <semaphore.h>

	#define PACKAGESIZE 1024 // Size máximo de paquete para sockets
	#define RUTA_CONFIG_UMC "configUMC.txt"

	// Estructuras
	typedef struct {
		int backlog;
		int puerto; // para conexiones de CPU y Núcleo
		char *ip_swap;
		int puerto_swap;
		int marcos;
		int marco_size;
		int marco_x_proceso;
		int entradas_tlb;
		int retardo;
	} t_configuracion;

	typedef struct {
		int pagina;
		int pid;
		int marco;
		int bit_presencia;
		int bit_uso;
		int bit_modificado;
	} tp_t;

	typedef struct {
		int pagina;
		int pid;
		int marco;
	} tlb_t;


	// Globales
	t_configuracion *config; // guarda valores del config
	int sockClienteDeSwap, sockServidor; // Se lo va a llamar a necesidad en distintas funciones
	int entradas_tp;
	void *memoria; // tha memory
	tp_t *tabla_paginas;
	tlb_t *tlb;
	sem_t mutex;

	// Cabeceras
	void abrirArchivoConfig(char *ruta); // Setea todos los valores de configuración
	void iniciarEstructuras(); // Crea memoria y estructuras de administracións
	void conectarConSwap(); // Se crea al principio. Luego se llama a necesidad
	void crearHilos(); // Crea hilos servidor y consola
	void servidor(); // Las conexiones de CPU y Núcleo se van a realizar por acá
	void consola(); // Entrada por stdin
	void crearHiloCliente(int *sockCliente); // Crea un hilo cliente al aceptar conexión
	void cliente(void* sockCliente); // Maneja pedidos del cliente
	void liberarEstructura();
	void liberarRecusos();
	int validar_cliente(char *id); // Valida que el cliente sea CPU o Nucleo
	int validar_servidor(char *id); // Valida que el servidor sea Swap
	int inciar_programa(int pid, int paginas); // Llama a agregar_tp y le avisa a Swap
	void agregar_tp(int pid, int paginas); // Agrega un proceso a la tabla de páginas
	int buscar_pagina(int pid, int pagina);
	void eliminar_pagina(int pid, int pagina);
	void reset_entrada(int pos);
	void iniciarTP();

#endif /* LIB_FUMC_H_ */

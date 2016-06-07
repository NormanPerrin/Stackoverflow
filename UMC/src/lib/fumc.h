#ifndef LIB_FUMC_H_
#define LIB_FUMC_H_

	#include <utilidades/sockets.h>
	#include <utilidades/general.h>
	#include <utilidades/comunicaciones.h>
	#include <commons/config.h>
	#include <commons/log.h>
	#include <pthread.h>
	#include <semaphore.h>

	#define MAX_CONSOLA 50
	#define MAX_CONEXIONES 30
	#define RUTA_CONFIG "configUMC.txt"

	// Estructuras
	typedef struct {
		int backlog;
		int puerto;
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

	// Interfaz
	typedef struct {
		int pid;
		int paginas;
		char *codigo;
	}__attribute__((packed)) inciarPrograma_t;

	typedef struct {
		int pagina;
		int offset;
		int tamanio;
	}__attribute__((packed)) leerBytes_t;

	typedef struct {
		int pagina;
		int offset;
		int tamanio;
		char *contenido;
	}__attribute__((packed)) escribirBytes_t;

	typedef struct {
		int pid;
	}__attribute__((packed)) finalizarPrograma_t;

	// Pedidos a Swap
	typedef struct {
		int pid;
		int pagina;
	} pedidoPagina_t;

	typedef struct {
		int fd;
		int pid;
	} pid_activo;


	// Globales
	t_configuracion *config; // guarda valores del config
	int sockClienteDeSwap, sockServidor; // Se lo va a llamar a necesidad en distintas funciones
	int entradas_tp;
	void *memoria; // tha memory
	tp_t *tabla_paginas;
	tlb_t *tlb;
	sem_t mutex;
	sem_t mutex_pid;
	pid_activo pids[MAX_CONEXIONES];

	// Cabeceras

	// <CONEXIONES_FUNCS>
	void conectarConSwap(); // Se crea al principio. Luego se llama a necesidad
	void crearHilos(); // Crea hilos servidor y consola
	void servidor(); // Las conexiones de CPU y Núcleo se van a realizar por acá
	void consola(); // Entrada por stdin
	void crearHiloCliente(int *sockCliente); // Crea un hilo cliente al aceptar conexión
	void cliente(void* sockCliente); // Maneja pedidos del cliente
	// </CONEXIONES_FUNCS>

	// <AUXILIARES>
	void abrirArchivoConfig(char *ruta); // Setea todos los valores de configuración
	void iniciarEstructuras(); // Crea memoria y estructuras de administracións
	void liberarEstructura();
	void liberarRecusos();
	int validar_cliente(char *id); // Valida que el cliente sea CPU o Nucleo
	int validar_servidor(char *id); // Valida que el servidor sea Swap
	void pedir_pagina(int fd, int pid, int pagina);
	void enviarTamanioMarco(int fd, int tamanio);
	void *elegirFuncion(protocolo head);
	void responder(int fd, int respuesta);
	int pedir_pagina_swap(int fd, int pid, int pagina);
	// </AUXILIARES>

	// <PRINCIPAL>
	void inciar_programa(int fd, void *msj); // Llama a agregar_tp y le avisa a Swap
	void leer_bytes(int fd, void *msj);
	void finalizar_programa(int fd, void *msj);
	void escribir_bytes(int fd, void *mensaje);
	// </PRINCIPAL>

	// <TABLA_PAGINA>
	void agregar_tp(int pid, int paginas); // Agrega un proceso a la tabla de páginas
	int buscar_tp(int pid, int pagina);
	void eliminar_pagina(int pid, int pagina);
	void reset_entrada(int pos);
	void iniciarTP();
	int cargar_pagina(int pid, int pagina, void *contenido);
	int buscarPagina(int fd, int pid, int pagina);
	void actualizar_tp(int pid, int pagina, int marco, int b_presencia, int b_modificacion, int b_uso);
	int contar_paginas(int pid);
	int contar_paginas_asignadas(int pid);
	// </TABLA_PAGINA>

	// <PID_FUNCS>
	void actualizarPid(int fd, int pid);
	int buscarPosPid(int fd);
	int buscarEspacioPid();
	void borrarPid(int fd);
	void agregarPid(int fd, int pid);
	// </PID_FUNCS>

	// <TLB_FUNCS>
	void actualizar_tlb(int pid, int pagina);
	void borrar_tlb(int pid, int pagina);
	int buscar_tlb(int pid, int pagina);
	// </TLB_FUNCS>

#endif /* LIB_FUMC_H_ */

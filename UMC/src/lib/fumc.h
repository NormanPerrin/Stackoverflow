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
	#define MAX_PROCESOS 100
	#define MAX_PAGINAS 30
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
		int marco;
		int bit_presencia;
		int bit_uso;
		int bit_modificado;
	} subtp_t;

	typedef struct {
		int pid;
		int puntero;
		int paginas;
		subtp_t tabla[MAX_PAGINAS];
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
	void *memoria; // tha memory
	tp_t *tabla_paginas[MAX_PROCESOS];
	tlb_t *tlb;
	sem_t mutex_tp;
	sem_t mutex_pid;
	sem_t mutex_tlb;
	sem_t mutex_mp;
	pid_activo pids[MAX_CONEXIONES];
	int *bitmap;

	// Cabeceras

	void setearValores_config(t_config * archivoConfig);

	// <CONEXIONES_FUNCS>
	void conectarConSwap();
	void crearHilos();
	void consola();
	void servidor();
	void crearHiloCliente(int *sockCliente);
	void cliente(void* fdCliente);
	int pedir_pagina_swap(int fd, int pid, int pagina);
	void enviarTamanioMarco(int fd, int tamanio);
	int validar_cliente(char *id);
	int validar_servidor(char *id);
	// </CONEXIONES_FUNCS>

	// <AUXILIARES>
	void iniciarEstructuras();
	void liberarConfig();
	void liberarRecusos();
	void *elegirFuncion(protocolo head);
	void compararProtocolos(int protocolo1, int protocolo2);
	// </AUXILIARES>

	// <PRINCIPAL>
	void inciar_programa(int fd, void *msj);
	void leer_bytes(int fd, void *msj);
	void escribir_bytes(int fd, void *msj);
	void finalizar_programa(int fd, void *msj);
	void cambiarPid(int fd, void *mensaje);
	// </PRINCIPAL>

	// <TABLA_PAGINA>
	void iniciarTP();
	void reset_entrada(int pos);
	void setear_entrada(int pos, int subpos, subtp_t set);
	int pos_pid(int pid);
	void agregar_paginas_nuevas(int pid, int paginas);
	void agregar_pagina_nueva(int pid, int pagina);
	int contar_paginas_asignadas(int pid);
	void eliminar_pagina(int pid, int pagina);
	int buscarPagina(int fd, int pid, int pagina);
	int cargar_pagina(int pid, int pagina, void *contenido);
	void actualizar_tp(int pid, int pagina, int marco, int b_presencia, int b_modificacion, int b_uso);
	// </TABLA_PAGINA>

	// <PID_FUNCS>
	int buscarPosPid(int fd);
	void actualizarPid(int fd, int pid);
	void agregarPid(int fd, int pid);
	int buscarEspacioPid();
	void borrarPid(int fd);
	// </PID_FUNCS>

	// <TLB_FUNCS>
	int buscar_tlb(int pid, int pagina);
	void borrar_tlb(int pid, int pagina);
	void actualizar_tlb(int pid, int pagina);
	// </TLB_FUNCS>

	// <MEMORIA_FUNCS>
	void borrarMarco(int marco);
	int buscarMarcoLibre();
	// </MEMORIA_FUNCS>

#endif /* LIB_FUMC_H_ */

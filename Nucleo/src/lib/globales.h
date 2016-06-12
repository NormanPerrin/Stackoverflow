#ifndef LIB_GLOBALES_H_
#define LIB_GLOBALES_H_

#include <utilidades/sockets.h>
#include <utilidades/general.h>
#include <utilidades/comunicaciones.h>
#include <pthread.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <semaphore.h>
#include <parser/metadata_program.h>
#include <sys/inotify.h>

#define EVENT_SIZE ( sizeof (struct inotify_event) + strlen(RUTA_CONFIG_NUCLEO) + 1 )
#define CONEXIONES_PERMITIDAS 10
#define PACKAGESIZE 1024 // Size máximo de paquete para sockets
#define RUTA_CONFIG_NUCLEO "/home/utnso/tp-2016-1c-Cazadores-de-cucos/Nucleo/configNucleo.txt"
#define logearError(msg){log_error(logger, msg); return FALSE;}

// Estructuras:
typedef struct {
	int puertoPrograma;
	int puertoCPU;
	int puertoUMC;
	char * ipUMC;
	int quantum;
	int retardoQuantum;
	char ** semaforosID;
	char** semaforosValInicial;
	char ** ioID;
	char** retardosIO;
	char ** variablesCompartidas;
	int cantidadPaginasStack;
} t_configuracion;

typedef struct {
	int id;
	int fd_consola;
	int pid;
	string programa;
} consola;

typedef enum {
	LIBRE, OCUPADO
} disponibilidadCPU;

typedef struct {
	int id;
	int fd_cpu;
	int disponibilidad;
	int pid;
} cpu;

// VARIABLES GLOBALES:
t_configuracion * config;
int fd_UMC;
t_log * logger;
t_list * listaProcesos; // listaNuevos
t_queue * colaListos;
t_queue * colaBloqueados;
t_list * listaCPU;
t_list * listaConsolas;
int tamanioPagina;

fd_set readfds;
int fdEscuchaConsola;
int fdEscuchaCPU;
int fd_inotify;

#endif /* LIB_GLOBALES_H_ */

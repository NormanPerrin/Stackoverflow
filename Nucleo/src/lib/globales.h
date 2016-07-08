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
#include <sys/inotify.h>
#include <parser/metadata_program.h>

#define EVENT_SIZE ( sizeof (struct inotify_event) + strlen(RUTA_CONFIG_NUCLEO) + 1 )
#define CONEXIONES_PERMITIDAS 10
#define PACKAGESIZE 1024 // Size máximo de paquete para sockets
#define RUTA_CONFIG_NUCLEO "/home/utnso/tp-2016-1c-Cazadores-de-cucos/Nucleo/configNucleo.txt"
#define logearError(msg){log_error(logger, msg); return FALSE;}

typedef struct {
	int puertoPrograma, puertoCPU, puertoUMC, quantum, retardoQuantum, cantidadPaginasStack;
	char * ipUMC;
	char ** semaforosID;
	char ** semaforosValInicial;
	char ** ioID;
	char ** retardosIO;
	char ** variablesCompartidas;
} t_configuracion;

typedef struct {
	char *nombre;
	int valor;
	t_queue *bloqueados;
} t_semaforo;

typedef struct {
	int id, fd_consola, pid;
	string programa;
} consola;

typedef enum {
	LIBRE, OCUPADO
} disponibilidadCPU;

typedef struct {
	int id, fd_cpu, disponibilidad, pid;
} cpu;

// VARIABLES GLOBALES:

t_configuracion * config;
int fd_UMC, tamanioPagina, fdEscuchaConsola, fdEscuchaCPU, fd_inotify;
t_log * logger;
fd_set readfds;

t_list * listaProcesos; // Lista de todos los procesos en el sistema
t_list * listaCPU; // Lista de todos las CPU conectadas
t_list * listaConsolas;  // Lista de todos las Consolas conectadas

t_queue * colaListos; // Lista de todos los procesos listos para ejecutar

t_dictionary * diccionarioIO; // Diccionario de todos los dispositivos IO
t_dictionary * diccionarioSemaforos; // Diccionario de todos los semáforos
t_dictionary * diccionarioVarCompartidas; // Diccionario de todos las variables compartidas

pthread_mutex_t mutex_planificarProceso;

#endif /* LIB_GLOBALES_H_ */

#ifndef LIB_FNUCLEO_H_
#define LIB_FNUCLEO_H_

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
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))
#define CONEXIONES_PERMITIDAS 10
#define PACKAGESIZE 1024 // Size máximo de paquete para sockets
#define RUTA_CONFIG_NUCLEO "configNucleo.txt"
#define logearError(msg){log_error(logger, msg); return FALSE;}

// -- Estructuras --
typedef struct {
		int puertoPrograma;
		int puertoCPU;
		int puertoUMC;
		char * ipUMC;
		int quantum;
		int retardoQuantum;
		char ** semaforosID;
		int * semaforosValInicial;
		char ** ioID;
		int * retardosIO;
		char ** variablesCompartidas;
		int cantidadPaginasStack;
	} t_configuracion;

typedef struct {
	int fd_consola;
	char* nombrePrograma;
} consola;

typedef struct {
	int fd_cpu;
	int disponibilidad;
} cpu;

typedef enum {
	LIBRE, OCUPADO
} disponibilidadCPU;

// -- Variables Globales --
t_configuracion * config;
int fd_serverUMC; // cliente de UMC
t_log * logger;
t_list * listaProcesos;
t_list * listaCPU;
t_list * listaConsolas;
t_queue * colaReady;
t_queue * colaBlock;
/*sem_t mutex_ready;
sem_t mutex_block;*/

// -- Cabeceras de Funciones --
void abrirArchivoDeConfiguracion(char * ruta);
void setearValores_config(t_config * archivoConfig);
void inicializarListas();

void conectarConUMC();
void escucharACPU();
void escucharAConsola();
void actualizarDatosEnPCBProceso(cpu * unCPU, pcb * pcbNuevo);

void crearLogger();

int asignarPid();
int noSeRepitePid(int pid);
pcb* buscarProcesoPorPid(int pid);
pcb* crearPCB(char * unPrograma);
void liberarPCB(pcb * pcb);
void planificarProceso();

pcb* readyAExec();
void execAReady();
void execABlock();
void blockAReady();
void newAReady();

// -- Funciones auxiliares --
int* convertirStringsEnNumeros(char ** variablesConfig);

#endif /* LIB_FNUCLEO_H_ */

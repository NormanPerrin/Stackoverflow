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
#include <parser/metadata_program.h>
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))
#define CONEXIONES_PERMITIDAS 10
#define PACKAGESIZE 1024 // Size máximo de paquete para sockets

#define RUTA_CONFIG_NUCLEO "/home/utnso/tp-2016-1c-Cazadores-de-cucos/Nucleo/configNucleo.txt"
//#define RUTA_CONFIG_NUCLEO "configNucleo.txt"
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
		char** semaforosValInicial;
		char ** ioID;
		char** retardosIO;
		char ** variablesCompartidas;
		int cantidadPaginasStack;
	} t_configuracion;

typedef struct {
	int fd_consola;
	int pid;
	string programa;
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
int fd_clienteUMC; // cliente de UMC
t_log * logger;
t_list * listaProcesos;
t_list * listaCPU;
t_list * listaConsolas;
t_queue * colaReady;
t_queue * colaBlock;
int tamanioPagina;
/*sem_t mutex_ready;
sem_t mutex_block;*/

// -- Prototipos de Funciones --
void abrirArchivoDeConfiguracion(char * ruta);
void setearValores_config(t_config * archivoConfig);
void inicializarListasYColas();

void conectarConUMC();
void escucharACPU();
void escucharAConsola();
void actualizarDatosDePCBEjecutada(cpu * unCPU, pcb * pcbNuevo);
void inicializarIndices(pcb* pcb, t_metadata_program* metaData);

void crearLogger();

int asignarPid(t_list procesos);
pcb* buscarProcesoPorPid(int pid, int* index);
pcb* crearPCB(string programa);
void liberarPcb(pcb * pcb);
void planificarProceso();
void finalizarPrograma(int pid);

pcb* readyAExec();
void execAReady();
void execABlock();
void blockAReady();
void newAReady();

void liberarTodaLaMemoria();
void limpiarListasYColas();
void liberarConsola(consola * consola);
void liberarCPU(cpu * cpu);

// -- Funciones auxiliares --

#endif /* LIB_FNUCLEO_H_ */

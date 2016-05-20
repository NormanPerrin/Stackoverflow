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
#include <commons/collections/dictionary.h>
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
	int id;
	int fd;
	int estado;
} cpu;

typedef enum {
	LIBRE, OCUPADO
} estadoCPU;

typedef enum {
	NEW, READY, EXEC, BLOCK, EXIT
} estadoProceso; // t_cola

// -- Variables Globales --
t_configuracion * config;
int fd_serverUMC; // cliente de UMC
t_log * logger;
t_list * listaProcesos;
t_list * listaProcesosListos;
t_list * listaProcesosBloqueados;
t_list * listaCPU;

// -- Cabeceras de Funciones --
void abrirArchivoDeConfiguracion(char * ruta);
void setearValores_config(t_config * archivoConfig);
void inicializarListas();

void conectarConUMC();
void crearHilosEscucharConsolaYCpu(); // 1 hilo escuchar_conexiones(CPU) y otro escuchar_conexiones(Consola)
void escuchar_conexiones(); // Escucha conexiones CPU(s) y Consola(s)

void crearLogger();

int asignarPid();
int noSeRepitePid(int pid);
pcb* crearPCB(char * unPrograma);
void liberarPCB(pcb * pcb);

// -- Funciones auxiliares --
int* convertirStringsEnNumeros(char ** variablesConfig);
int validar_cliente(char *id); // Valida que el cliente sea CPU o Consola
int validar_servidor(char *id); // Valida que el servidor sea UMC

#endif /* LIB_FNUCLEO_H_ */

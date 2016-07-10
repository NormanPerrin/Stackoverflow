#ifndef LIB_PRINCIPALES_H_
#define LIB_PRINCIPALES_H_

#include "funciones.h"
#include "globales.h"
#include "entradaSalida.h"
#include "semaforos.h"

/*** FUNCIONES PRINCIPALES ***/
int init_ok();
int leerConfiguracionNucleo();
int crearLoggerNucleo();
int iniciarEscuchaDeInotify();
void inicializarColecciones();
void llenarDiccionarioSemaforos();
void llenarDiccionarioVarCompartidas();
void lanzarHilosIO();
int conectarConUMC();
int obtenerSocketMaximoInicial();
int iniciarEscuchaDeConsolasYCPUs();
int crearThreadPlanificacion();
void esperar_y_PlanificarProgramas();
void unirHilosIO();
void liberarRecursosUtilizados();

/*** INCLUÍDAS ***/
void aceptarConexionEntranteDeConsola();
void aceptarConexionEntranteDeCPU();
void atenderNuevoMensajeDeCPU();
void atenderCambiosEnArchivoConfig(int socketMaximo);
var_compartida* crearVariableCompartida(char* nombre, int valorInicial);

#endif /* LIB_PRINCIPALES_H_ */

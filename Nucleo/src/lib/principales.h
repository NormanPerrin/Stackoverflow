#ifndef LIB_PRINCIPALES_H_
#define LIB_PRINCIPALES_H_

#include "funciones.h"
#include "globales.h"
#include "entradaSalida.h"
#include "semaforos.h"

/*** FUNCIONES PRINCIPALES ***/
void crearLoggerNucleo();
void inicializarColecciones();
void llenarDiccionarioSemaforos();
void llenarDiccionarioVarCompartidas();
void lanzarHilosIO();
void conectarConUMC();
int obtenerSocketMaximoInicial();
void iniciarEscuchaDeConsolasYCPUs();
void esperar_y_PlanificarProgramas();
void unirHilosIO();
void liberarRecursosUtilizados();
void iniciarEscuchaDeInotify();

/*** INCLUÍDAS ***/
void aceptarConexionEntranteDeConsola();
void aceptarConexionEntranteDeCPU();
void atenderNuevoMensajeDeCPU();
void atenderCambiosEnArchivoConfig(int socketMaximo);
var_compartida* crearVariableCompartida(char* nombre, int valorInicial);

#endif /* LIB_PRINCIPALES_H_ */

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
void iniciarEscuchaConsolasYCPUs();
void esperar_y_PlanificarProgramas();
void unirHilosIO();
void liberarRecursosUtilizados();

/*** INCLUÍDAS ***/
void aceptarConexionEntranteDeConsola();
void aceptarConexionEntranteDeCPU();
void atenderNuevoMensajeDeCPU();
var_compartida* crearVariableCompartida(char* nombre, int valorInicial);

#endif /* LIB_PRINCIPALES_H_ */

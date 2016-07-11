#ifndef LIB_PRINCIPALES_H_
#define LIB_PRINCIPALES_H_

#include "funciones.h"
#include "globales.h"
#include "entradaSalida.h"
#include "semaforos.h"

/*** FUNCIONES PRINCIPALES ***/
void inicializarColecciones();
void crearLoggerNucleo();
void leerConfiguracionNucleo();
void iniciarEscuchaDeInotify();
void llenarDiccionarioSemaforos();
void llenarDiccionarioVarCompartidas();
void lanzarHilosIO();
int conectarConUMC();
void esperar_y_PlanificarProgramas();
void unirHilosIO();
void liberarRecursosUtilizados();
/*** INCLUÍDAS ***/
int obtenerSocketMaximoInicial();
void aceptarConexionEntranteDeConsola();
void aceptarConexionEntranteDeCPU();
void recorrerListaCPUsYAtenderNuevosMensajes();
void atenderCambiosEnArchivoConfig(int socketMaximo);

#endif /* LIB_PRINCIPALES_H_ */

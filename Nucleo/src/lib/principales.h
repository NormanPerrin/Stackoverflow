#ifndef LIB_PRINCIPALES_H_
#define LIB_PRINCIPALES_H_

#include "funciones.h"

/*** FUNCIONES PRINCIPALES ***/
void inicializarColecciones();
void crearLoggerNucleo();
void leerConfiguracionNucleo();
void llenarDiccionarioSemaforos();
void llenarDiccionarioVarCompartidas();
void lanzarHilosIO();
int conexionConUMC();
void esperar_y_PlanificarProgramas();
void unirHilosIO();
void liberarRecursosUtilizados();
int validar_cliente();
int validar_servidor();

extern bool seDesconectoUMC; // Variable global proveniente del main

#endif /* LIB_PRINCIPALES_H_ */

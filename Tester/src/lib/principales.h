/*
 * principales.h
 *
 *  Created on: 13/7/2016
 *      Author: utnso
 */

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
void exitNucleo();

#endif /* LIB_PRINCIPALES_H_ */

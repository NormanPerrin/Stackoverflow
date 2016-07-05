#ifndef LIB_PRINCIPALES_H_
#define LIB_PRINCIPALES_H_

#include "funciones.h"
#include "globales.h"
#include "entradaSalida.h"
#include "semaforos.h"
#include "variablesCompartidas.h"

/**** FUNCIONES PRINCIPALES ****/
void abrirArchivoDeConfiguracion(char * ruta);
void inicializarListasYColas();
void llenarDiccionarioSemaforos();
void llenarDiccionarioVarCompartidas();
void conectarConUMC();
void esperar_y_PlanificarProgramas();
void liberarTodaLaMemoria();

// INCLUIDAS:
void aceptarConexionEntranteDeConsola();
void aceptarConexionEntranteDeCPU();
var_compartida* crearVariableCompartida(char* nombre, int valorInicial);

#endif /* LIB_PRINCIPALES_H_ */

#ifndef LIB_PRINCIPALES_H_
#define LIB_PRINCIPALES_H_

#include "funciones.h"
#include "globales.h"

/**** FUNCIONES PRINCIPALES ****/
void abrirArchivoDeConfiguracion(char * ruta);
void inicializarListasYColas();
void conectarConUMC();
void esperar_y_PlanificarProgramas();
void escucharCPUs_y_Planificar();
void liberarTodaLaMemoria();

#endif /* LIB_PRINCIPALES_H_ */

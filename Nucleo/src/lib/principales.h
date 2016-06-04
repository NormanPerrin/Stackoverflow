#ifndef LIB_PRINCIPALES_H_
#define LIB_PRINCIPALES_H_

#include "funciones.h"
#include "globales.h"

/**** FUNCIONES PRINCIPALES ****/
void abrirArchivoDeConfiguracion(char * ruta);
void inicializarListasYColas();
void conectarConUMC();
void escucharAConsola();
void escucharACPU();
void liberarTodaLaMemoria();

#endif /* LIB_PRINCIPALES_H_ */

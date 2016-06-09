#ifndef LIB_PRINCIPALESCPU_H_
#define LIB_PRINCIPALESCPU_H_

#include "secundariasCPU.h"

/** PROTOTIPO FUNCIONES PRINCIPALES **/
void leerArchivoDeConfiguracion(char * ruta);
void conectarConUMC();
void conectarConNucleo();
void liberarEstructura(); // Libera la memoria reservada en setear config

#endif

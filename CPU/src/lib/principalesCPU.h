#ifndef LIB_PRINCIPALESCPU_H_
#define LIB_PRINCIPALESCPU_H_

#include "fcpu.h"

/** PROTOTIPO FUNCIONES PRINCIPALES **/
leerArchivoDeConfiguracion(char * ruta);
conectarConUMC();
conectarConNucleo();
void liberarEstructura(); // Libera la memoria reservada en setear config

#endif

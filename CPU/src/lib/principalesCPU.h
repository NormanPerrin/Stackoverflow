#ifndef LIB_PRINCIPALESCPU_H_
#define LIB_PRINCIPALESCPU_H_

#include "secundariasCPU.h"

/** PROTOTIPO FUNCIONES PRINCIPALES **/
void crearLogger();
void conectarConUMC();
void obtenerTamanioDePagina();
void conectarConNucleo();
void ejecutarProcesos();
void liberarEstructuras(); // Libera la memoria reservada en setear config
void ejecutarProcesoActivo();
void ejecutarInstruccion(t_intructions instruccionActual);

#endif

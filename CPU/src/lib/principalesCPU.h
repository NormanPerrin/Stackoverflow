#ifndef LIB_PRINCIPALESCPU_H_
#define LIB_PRINCIPALESCPU_H_

#include "secundariasCPU.h"

/** PROTOTIPO FUNCIONES PRINCIPALES **/
void crearLoggerCPU();
void atenderSenialSIGUSR1(int value);
int conectarConUMC();
void obtenerTamanioDePagina();
void conectarConNucleo();
char* solicitarProximaInstruccionAUMC();
void liberarRecursos();
// En el main:
int recibirMensajesDeNucleo();
void ejecutarProcesoActivo();

#endif

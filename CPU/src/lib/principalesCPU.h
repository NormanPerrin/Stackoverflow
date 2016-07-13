#ifndef LIB_PRINCIPALESCPU_H_
#define LIB_PRINCIPALESCPU_H_

#include "primitivasAnSISOP.h"

// En el main:
int recibirMensajesDeNucleo();
void ejecutarProcesoActivo();
// En principales.c:
void crearLoggerCPU();
void setearValores_config(t_config * archivoConfig);
void liberarPcbActiva();
void atenderSenialSIGUSR1();
int conectarConUMC();
void obtenerTamanioDePagina();
void conectarConNucleo();
void revisarFinalizarCPU();
int recibirYvalidarEstadoDelPedidoAUMC();
void limpiarInstruccion(char * instruccion);
char* solicitarProximaInstruccionAUMC();
void liberarRecursos();

#endif

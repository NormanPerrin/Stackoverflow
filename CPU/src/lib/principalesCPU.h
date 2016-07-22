#ifndef CPU_SRC_LIB_PRINCIPALESCPU_H_
#define CPU_SRC_LIB_PRINCIPALESCPU_H_

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
void exitPorErrorUMC();
void exitProceso();
void limpiarInstruccion(char * instruccion);
void liberarRegistroStack(registroStack* reg);
char* solicitarProximaInstruccionAUMC();
void liberarRecursos();
void exitCPU();
// conexiones
int validar_cliente(char *id);
int validar_servidor(char *id);

#endif

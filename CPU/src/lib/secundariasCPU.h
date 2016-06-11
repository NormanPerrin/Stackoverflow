#ifndef LIB_SECUNDARIASCPU_H_
#define LIB_SECUNDARIASCPU_H_

#include "globalesCPU.h"
#include "primitivasAnSISOP.h"

/** PROTOTIPO FUNCIONES SECUNDARIAS **/
void setearValores_config(t_config * archivoConfig);
void crearLogger();
void ejecutarInstruccion(pcb* pcb);
void ejecutarProceso(pcb* pcb);
int validar_servidor(char *id); // Valida si la conexión es UMC o Nucleo
int validar_cliente(char *id); // para que no joda con error
char* charToString(char element);

#endif /* LIB_SECUNDARIASCPU_H_ */

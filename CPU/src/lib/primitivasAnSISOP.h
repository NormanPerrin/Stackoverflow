#ifndef CPU_SRC_LIB_PRIMITIVASANSISOP_H_
#define CPU_SRC_LIB_PRIMITIVASANSISOP_H_

#include "globalesCPU.h"
#include "principalesCPU.h"

/** PROTOTIPO PRIMITIVAS ANSISOP **/
bool esArgumento(t_nombre_variable identificador_variable);

t_puntero definirVariable(t_nombre_variable identificador_variable);
t_puntero obtenerPosicionVariable(t_nombre_variable identificador_variable);
t_valor_variable dereferenciar(t_puntero direccion_variable);
void asignar(t_puntero direccion_variable, t_valor_variable valor);
void irAlLabel(t_nombre_etiqueta t_nombre_etiqueta);
void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar);
void retornar(t_valor_variable retorno);
void imprimir(t_valor_variable valor_mostrar);
void imprimirTexto(char* texto);
t_valor_variable obtenerValorCompartida(t_nombre_compartida variable);
t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor);
void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo);
void s_wait(t_nombre_semaforo identificador_semaforo);
void s_signal(t_nombre_semaforo identificador_semaforo);
//void finalizar(void);

// Variables globales provenientes del main:
extern bool finalizarCPU;
extern bool cpuOciosa;
extern bool huboStackOverflow;
extern int devolvioPcb;

#endif /* CPU_SRC_LIB_PRIMITIVASANSISOP_H_ */

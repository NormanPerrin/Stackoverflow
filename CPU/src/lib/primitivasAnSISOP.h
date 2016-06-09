#ifndef LIB_PRIMITIVASANSISOP_H_
#define LIB_PRIMITIVASANSISOP_H_

#include "globalesCPU.h"

/** PROTOTIPO PRIMITIVAS ANSISOP **/
t_puntero definirVariable(t_nombre_variable identificador_variable);
t_puntero obtenerPosicionVariable(t_nombre_variable identificador_variable);
t_valor_variable dereferenciar(t_puntero direccion_variable);
void asignar(t_puntero direccion_variable, t_valor_variable valor);
t_valor_variable obtenerValorCompartida(t_nombre_compartida variable);
t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor);
void irAlLabel(t_nombre_etiqueta t_nombre_etiqueta);
void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar);
void retornar(t_valor_variable retorno);
void imprimirAnSISOP(t_valor_variable valor_mostrar);
void imprimirTexto(char* texto);
void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo);
void wait(t_nombre_semaforo identificador_semaforo);
void signal(t_nombre_semaforo identificador_semaforo);

// Estructuras funcionesAnSISOP
AnSISOP_funciones funcionesAnSISOP = {
		.AnSISOP_definirVariable			= definirVariable,
		.AnSISOP_obtenerPosicionVariable	= obtenerPosicionVariable,
		.AnSISOP_dereferenciar				= dereferenciar,
		.AnSISOP_asignar					= asignar,
		.AnSISOP_obtenerValorCompartida		= obtenerValorCompartida,
		.AnSISOP_asignarValorCompartida		= asignarValorCompartida,
		.AnSISOP_irAlLabel					= irAlLabel,
		.AnSISOP_llamarConRetorno			= llamarConRetorno,
		.AnSISOP_retornar					= retornar,
		.AnSISOP_imprimir					= imprimirAnSISOP,
		.AnSISOP_imprimirTexto				= imprimirTexto,
};
AnSISOP_kernel funcionesKernel = {
		.AnSISOP_wait		= wait,
		.AnSISOP_signal 	= signal,
};


#endif /* LIB_PRIMITIVASANSISOP_H_ */

#ifndef LIB_PRIMITIVASANSISOP_H_
#define LIB_PRIMITIVASANSISOP_H_

#include "globalesCPU.h"
#include <commons/collections/dictionary.h>

/** PROTOTIPO PRIMITIVAS ANSISOP **/
t_puntero AnSISOP_definirVariable(t_nombre_variable identificador_variable);
t_puntero AnSISOP_obtenerPosicionVariable(t_nombre_variable identificador_variable);
t_valor_variable AnSISOP_dereferenciar(t_puntero direccion_variable);
void AnSISOP_asignar(t_puntero direccion_variable, t_valor_variable valor);
void AnSISOP_irAlLabel(t_nombre_etiqueta t_nombre_etiqueta);
void AnSISOP_llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar);
void AnSISOP_retornar(t_valor_variable retorno);
void AnSISOP_imprimir(t_valor_variable valor_mostrar);
void AnSISOP_imprimirTexto(char* texto);

t_valor_variable AnSISOP_obtenerValorCompartida(t_nombre_compartida variable);
t_valor_variable AnSISOP_asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor);
void AnSISOP_entradaSalida(t_nombre_dispositivo dispositivo, int tiempo);
void AnSISOP_wait(t_nombre_semaforo identificador_semaforo);
void AnSISOP_signal(t_nombre_semaforo identificador_semaforo);

// Estructuras funcionesAnSISOP
AnSISOP_funciones funcionesAnSISOP = {
		.AnSISOP_definirVariable			= AnSISOP_definirVariable,
		.AnSISOP_obtenerPosicionVariable	= AnSISOP_obtenerPosicionVariable,
		.AnSISOP_dereferenciar				= AnSISOP_dereferenciar,
		.AnSISOP_asignar					= AnSISOP_asignar,
//		.AnSISOP_obtenerValorCompartida		= AnSISOP_obtenerValorCompartida,
//		.AnSISOP_asignarValorCompartida		= AnSISOP_asignarValorCompartida,
		.AnSISOP_irAlLabel					= AnSISOP_irAlLabel,
		.AnSISOP_llamarConRetorno			= AnSISOP_llamarConRetorno,
		.AnSISOP_retornar					= AnSISOP_retornar,
		.AnSISOP_imprimir					= AnSISOP_imprimir,
		.AnSISOP_imprimirTexto				= AnSISOP_imprimirTexto,
//		.AnSISOP_entradaSalida				= AnSISOP_entradaSalida,
};

AnSISOP_kernel funcionesKernel = {
//		.AnSISOP_wait		= AnSISOP_wait,
//		.AnSISOP_signal 	= AnSISOP_signal,
};

#endif /* LIB_PRIMITIVASANSISOP_H_ */

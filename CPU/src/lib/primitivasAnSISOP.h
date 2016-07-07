#ifndef LIB_PRIMITIVASANSISOP_H_
#define LIB_PRIMITIVASANSISOP_H_

#include "globalesCPU.h"

/** PROTOTIPO PRIMITIVAS ANSISOP **/
t_puntero AnSISOP_DefinirVariable(t_nombre_variable identificador_variable);
t_puntero AnSISOP_ObtenerPosicionVariable(t_nombre_variable identificador_variable);
t_valor_variable AnSISOP_Dereferenciar(t_puntero direccion_variable);
void AnSISOP_Asignar(t_puntero direccion_variable, t_valor_variable valor);
t_puntero_instruccion AnSISOP_IrAlLabel(t_nombre_etiqueta t_nombre_etiqueta);
void AnSISOP_LlamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar);
void AnSISOP_Retornar(t_valor_variable retorno);
void AnSISOP_Imprimir(t_valor_variable valor_mostrar);
void AnSISOP_ImprimirTexto(char* texto);

t_valor_variable AnSISOP_ObtenerValorCompartida(t_nombre_compartida variable);
t_valor_variable AnSISOP_AsignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor);
void AnSISOP_EntradaSalida(t_nombre_dispositivo dispositivo, int tiempo);
void AnSISOP_Wait(t_nombre_semaforo identificador_semaforo);
void AnSISOP_Signal(t_nombre_semaforo identificador_semaforo);

// Estructuras funcionesAnSISOP
AnSISOP_funciones funcionesAnSISOP = {
		.AnSISOP_definirVariable			= AnSISOP_DefinirVariable,
		.AnSISOP_obtenerPosicionVariable	= AnSISOP_ObtenerPosicionVariable,
		.AnSISOP_dereferenciar				= AnSISOP_Dereferenciar,
		.AnSISOP_asignar					= AnSISOP_Asignar,
		.AnSISOP_obtenerValorCompartida		= AnSISOP_ObtenerValorCompartida,
		.AnSISOP_asignarValorCompartida		= AnSISOP_AsignarValorCompartida,
		.AnSISOP_irAlLabel					= AnSISOP_IrAlLabel,
		.AnSISOP_llamarConRetorno			= AnSISOP_LlamarConRetorno,
		.AnSISOP_retornar					= AnSISOP_Retornar,
		.AnSISOP_imprimir					= AnSISOP_Imprimir,
		.AnSISOP_imprimirTexto				= AnSISOP_ImprimirTexto,
		.AnSISOP_entradaSalida				= AnSISOP_EntradaSalida,
};

AnSISOP_kernel funcionesKernel = {
		.AnSISOP_wait		= AnSISOP_Wait,
		.AnSISOP_signal 	= AnSISOP_Signal,
};

#endif /* LIB_PRIMITIVASANSISOP_H_ */

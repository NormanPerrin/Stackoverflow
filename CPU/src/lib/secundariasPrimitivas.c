/*
 * secundariasPrimitivas.c
 *
 *  Created on: 10/6/2016
 *      Author: utnso
 */
#include "secundariasPrimitivas.h"

t_puntero ultimaPosicionDeVariable(registroStack * stack){
	//mmm no puede ser esto :/ //
	return (stack->listaVariablesLocales->posicion);
}

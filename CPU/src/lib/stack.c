#include "stack.h"


registroStack* inicializarStack(){
		registroStack* nuevoStack =(registroStack*)malloc(sizeof(registroStack));
		//ver si hay que inicializar mas cosas al stack//
		return nuevoStack;
}

void push(pcb* pcbEnEjecucion,void* datos,t_size data_size) {
/*pcbEnEjecucion->indiceStack->listaVariablesLocales=*/
}
void pop(registroStack* stack){

}

t_puntero ultimaPosicionDeVariable(registroStack* stack) {
	return (stack->stack_pointer + stack_offsetFromContext(stack)- tamanio_registro_variable);
}


#ifndef LIB_STACK_H_
#define LIB_STACK_H_


/*** PROTOTIPO FUNCIONES STACK ***/

registroStack* inicializarStack();
void push(pcb* pcbEnEjecucion,void* datos,t_size data_size);
void pop();
t_puntero ultimaPosicionDeVariable();
#endif /* LIB_STACK_H_ */

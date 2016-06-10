#include "primitivasAnSISOP.h"
#include "principalesCPU.h"
//HACER
t_puntero definirVariable(t_nombre_variable nombre){

	char* identificador =charToString(nombre);
	push(PCBEJECUTANDO,&nombre,sizeof(nombre));
	t_puntero* posicion =malloc(sizeof(posicion));
	posicion=ultimaPosicionDeVariable(&(PCBEJECUTANDO->indiceStack));
	//hacer una funcion para agregar el identificador y la posicion al registroStack//
	agregarVariableAlIndiceDeStack(,identificador,posicion);

	free (identificador);

	return posicion+1;

}

//HACER
t_puntero obtenerPosicionVariable(t_nombre_variable identificador_variable){
	t_puntero posicion;

	return posicion;
}

//HACER
t_valor_variable dereferenciar(t_puntero direccion_variable){
	t_valor_variable valorVariable;

	return valorVariable;
}

//HACER
void asignar(t_puntero direccion_variable, t_valor_variable valor){

}

//NO HACER
t_valor_variable obtenerValorCompartida(t_nombre_compartida variable){
	t_valor_variable valorVariable;

	return valorVariable;
}

//NO HACER
t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){
	t_valor_variable valorVariable;

	return valorVariable;
}

//HACER
void irAlLabel(t_nombre_etiqueta t_nombre_etiqueta){

}

//HACER
void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){

}

//HACER
void retornar(t_valor_variable retorno){

}

//HACER
void imprimir(t_valor_variable valor_mostrar){

}

//HACER
void imprimirTexto(char* texto){

}

//HACER
void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo){

}

//HACER
void wait(t_nombre_semaforo identificador_semaforo){

}

//HACER
void signal(t_nombre_semaforo identificador_semaforo){

}


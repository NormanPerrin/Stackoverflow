#include "primitivasAnSISOP.h"
#include "principalesCPU.h"

//HACER
t_puntero definirVariable(t_nombre_variable nombre){

	char* identificador = malloc(sizeof(char));
	identificador = charToString((char)nombre);
	t_puntero * pos = malloc(sizeof(t_puntero));
	* pos = ultimaPosicionDeVariable(&(pcbActual->indiceStack->listaVariablesLocales));
	dictionary_put(pcbActual->indiceStack->listaVariablesLocales,identificador,pos);

	free (identificador);
	//return pos+1;//
	return (*(pos +1));
}

t_puntero obtenerPosicionVariable(t_nombre_variable nombre){
	char * identificador = malloc(sizeof(char));
	identificador = charToString((char)nombre);
	t_puntero * posicion = dictionary_get(pcbActual->indiceStack->listaVariablesLocales, identificador);
	free(identificador);
	if(posicion != NULL){
		return (*(posicion +1));
	}
	else{
		return -1;
	}
}

//HACER
t_valor_variable dereferenciar(t_puntero direccion){
	aplicar_protocolo_enviar(fdUMC, PEDIDO_LECTURA, &direccion);
	int protocolo;
	int valorVariable = aplicar_protocolo_recibir(fdUMC, &protocolo);
	respuestaPedido * respuesta = (respuestaPedido *) valorVariable;
	return ((t_valor_variable)respuesta->dataPedida.cadena);
}

//HACER
void asignar(t_puntero direccionVariable, t_valor_variable valor){
	direccion direccion = direccionVariable;
	solicitudEscritura * solicitud;
	solicitud->pagina = direccion.pagina;
	solicitud->offset = direccion.offset;
	solicitud->tamanio = direccion.size;
	solicitud->contenido = valor;

	aplicar_protocolo_enviar(fdUMC, PEDIDO_ESCRITURA, &solicitud);
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


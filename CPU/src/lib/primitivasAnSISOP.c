#include "primitivasAnSISOP.h"
#include "principalesCPU.h"

//ARREGLAR(VER COMO OBTENER LA POSICION)
t_puntero AnSISOP_definirVariable(t_nombre_variable nombre){

	char* identificador = malloc(sizeof(char));
	identificador = charToString((char)nombre);
	t_puntero * pos = malloc(sizeof(t_puntero));
	*pos = ultimaPosicionDeVariable(&(pcbActual->indiceStack->listaVariablesLocales));
	dictionary_put(pcbActual->indiceStack->listaVariablesLocales,identificador,pos);

	free (identificador);
	//return pos+1;//
	return (*(pos +1));
}

//HECHA(REVISAR SI ESTA BIEN)
t_puntero AnSISOP_obtenerPosicionVariable(t_nombre_variable nombre){
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

//HECHA(REVISAR SI ESTA BIEN)
t_valor_variable AnSISOP_dereferenciar(t_puntero direccion){
	int * direccionVar = malloc(INT);
	direccionVar = direccion;
	aplicar_protocolo_enviar(fdUMC, PEDIDO_LECTURA, &direccionVar);
	free(direccionVar);
	int protocolo;
	void * valorVariable = aplicar_protocolo_recibir(fdUMC, &protocolo);
	if(protocolo == RESPUESTA_PEDIDO){
	respuestaPedido * respuesta = (respuestaPedido *) valorVariable;
	t_valor_variable valor = atoi(respuesta->dataPedida.cadena);
	return valor;
	}
}

//ARREGLAR(VER COMO PASAR DE UN TIPO T_PUNTERO A TIPO DIRECCION)
void AnSISOP_asignar(t_puntero direccionVariable, t_valor_variable valor){
	direccion direccion = direccionVariable;
	solicitudEscritura * solicitud;
	pcbActual->indiceStack->posicionDelResultado.
	solicitud->pagina = direccion.pagina;
	solicitud->offset = direccion.offset;
	solicitud->tamanio = direccion.size;
	solicitud->contenido = valor;

	aplicar_protocolo_enviar(fdUMC, PEDIDO_ESCRITURA, &solicitud);
}

//HACER
t_valor_variable AnSISOP_obtenerValorCompartida(t_nombre_compartida variable){
	t_valor_variable valorVariable;

	return valorVariable;
}

//HACER
t_valor_variable AnSISOP_asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){
	t_valor_variable valorVariable;

	return valorVariable;
}

//HACER
void AnSISOP_irAlLabel(t_nombre_etiqueta t_nombre_etiqueta){

}

//HACER
void AnSISOP_llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){

}

//HACER
void AnSISOP_retornar(t_valor_variable retorno){

}

//ARREGLAR
void AnSISOP_imprimir(t_valor_variable valor_mostrar){
	int * valor = malloc(INT);
	* valor = valor_mostrar;
	aplicar_protocolo_enviar(fdNucleo,IMPRIMIR, valor);
	free(valor);
}

//HECHA(REVISAR SI ESTA BIEN)
void AnSISOP_imprimirTexto(char* texto){
	string * txt = malloc(STRING);
	txt->cadena = strdup(texto);
	txt->tamanio = strlen(texto) + 1;
	aplicar_protocolo_enviar(fdNucleo, IMPRIMIR_TEXTO, txt);
	free(txt->cadena);
	free(txt);
}

//HACER
void AnSISOP_entradaSalida(t_nombre_dispositivo dispositivo, int tiempo){

}

//HACER
void AnSISOP_wait(t_nombre_semaforo identificador_semaforo){

}

//HACER
void AnSISOP_signal(t_nombre_semaforo identificador_semaforo){

}


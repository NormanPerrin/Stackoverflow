#include "fnucleo.h"

// Funciones

void setearValores_config(t_config * archivoConfig){

	puertoPrograma = config_get_int_value(archivoConfig, "PUERTO_PROG");
	puertoCPU = config_get_int_value(archivoConfig, "PUERTO_CPU");
	quantum = config_get_int_value(archivoConfig, "QUANTUM");
	retardoQuantum = config_get_int_value(archivoConfig, "QUANTUM_SLEEP");
	pasarCadenasArray(semaforosID, config_get_array_value(archivoConfig, "SEM_IDS"));
	pasarCadenasArray(ioID, config_get_array_value(archivoConfig, "IO_IDS"));
	pasarCadenasArray(variablesCompartidas, config_get_array_value(archivoConfig, "SEHARED_VARS"));
	pasarEnterosArray(semaforosValInicial, config_get_array_value(archivoConfig, "SEM_INIT"));
	pasarEnterosArray(retardosIO, config_get_array_value(archivoConfig, "IO_SLEEP"));

}

void pasarCadenasArray(char** cadenas, char** variablesConfig){
	char* cadenaAux;
	int cantidadCadenas, i;

	cantidadCadenas = NELEMS(variablesConfig);

	for(i=0; i<(cantidadCadenas-1); i++){
		cadenaAux = variablesConfig[0];
		// Creo una nueva cadena y asigno su direccion a cadenas[i]
		cadenas[i] = (char*) malloc(strlen(cadenaAux+1));
		// Copio el contenido de cadenaAux a cadenas[i]
		strcpy(cadenas[i],cadenaAux);
		}
	}


void pasarEnterosArray(int* numeros, char** variablesConfig){
	int cantidadNumeros, i;

	cantidadNumeros = NELEMS(variablesConfig);

	for(i=0; i<(cantidadNumeros-1); i++){
		numeros[i] = atoi(variablesConfig[i]);
	}
}

void escucharCPU(){

}

void escucharConsola(){

}

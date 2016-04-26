#include "fnucleo.h"

// Funciones

void setearValores_config(t_config * archivoConfig){

	puertoPrograma = config_get_int_value(archivoConfig, "PUERTO_PROG");
	puertoCPU = config_get_int_value(archivoConfig, "PUERTO_CPU");
	quantum = config_get_int_value(archivoConfig, "QUANTUM");
	retardoQuantum = config_get_int_value(archivoConfig, "QUANTUM_SLEEP");

	// Tengo que ver cómo hago con estos arrays
	/*memcpy(&semaforosID, &(config_get_array_value(archivoConfig, "SEM_IDS")), sizeof semaforosID);
	memcpy(&semaforosValInicial, &(config_get_array_value(archivoConfig, "SEM_INIT")), sizeof semaforosValInicial);
	memcpy(&ioID, &(config_get_array_value(archivoConfig, "IO_IDS")), sizeof ioID);
	memcpy(&retardosIO, &(config_get_array_value(archivoConfig, "IO_SLEEP")), sizeof retardosIO);
	memcpy(&variablesCompartidas, &(config_get_array_value(archivoConfig, "SEHARED_VARS")), sizeof variablesCompartidas);*/

}

void escucharCPU(){

}

void escucharConsola(){

}

#include "general.h"
#include <stdlib.h>
#include <stdio.h>

void* reservarMemoria(int size) {
	void* puntero = malloc(size);
	if(puntero == NULL) {
		fprintf(stderr, "Error al reservar memoria\n");
		exit(-1);
	}
	return puntero;
}


void leerArchivoDeConfiguracion(char * ruta) {
	t_config * archivoConfig;

	if (comprobarQueExistaArchivo(ruta) == ERROR)
		manejarError("Error: Archivo de configuración no encontrado\n");

	printf("El archivo de configuración ha sido leído correctamente\n");

	archivoConfig = config_create(ruta);
	setearValores_config(archivoConfig); // Redefinido en cada proceso (Ejemplo en Núcleo)
	config_destroy(archivoConfig); // Destruye la estructura auxiliar config
}

int comprobarQueExistaArchivo(char* ruta) {
	FILE * archivoConfig = fopen(ruta, "r");
	if (archivoConfig!=NULL){
		fclose(archivoConfig);
		return TRUE;
	}
	return ERROR;
}


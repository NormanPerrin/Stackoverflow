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

	if (comprobarQueExistaArchivo(ruta) == ERROR){
		manejarError("Error: Archivo de configuración no encontrado\n");
	}else{

	archivoConfig = config_create(ruta);
	setearValores_config(archivoConfig); // Redefinido en cada proceso (Ejemplo en Núcleo)
	config_destroy(archivoConfig); // Libero la estructura archivoConfig

	printf("El archivo de configuración ha sido leído correctamente\n");
	}
}

int comprobarQueExistaArchivo(char *ruta) {
	if(access(ruta, F_OK) != ERROR){
		return TRUE; // Existe el archivo
	}
	return ERROR; // El archivo no existe
}

/*int comprobarQueExistaArchivo(char* ruta){
	FILE * archivoConfig = fopen(ruta, "r");
	if (archivoConfig!=NULL){
		fclose(archivoConfig);
		return TRUE;
	}
	return ERROR;
}*/

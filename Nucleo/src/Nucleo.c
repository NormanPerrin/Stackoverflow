#include <stdio.h>
#include <stdlib.h>
#include <utilidades/general.h>
#include "lib/fnucleo.h"

int main(void) {

	leerArchivoDeConfiguracion(RUTA_CONFIG_NUCLEO); // Abro archivo configuración

//	testLecturaArchivoDeConfiguracion(); // Comparo con los valores de ejemplo del archivo

	conectarConUMC(); // Conexión con UMC

	crear_hilos_conexion(); // Me pongo a escuchar conexiones CPU y Consola

	return EXIT_SUCCESS;
}

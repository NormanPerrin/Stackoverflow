#include <stdlib.h>
#include "lib/fconsola.h"

int main(int argc, char **argv) {

	system("clear");
	setvbuf(stdout, NULL, _IONBF, 0);

	//	validar_argumentos(argc); // Se le debe ingresar la ruta del script. Aunque se ejecute por #!

	leerArchivoDeConfiguracion(RUTA_CONFIG_CONSOLA);

	leerScript(argv[1]);

	conectarConNucleo(); // Conexión con Núcleo

	liberarRecusos(); // Libera memoria asignada

	return EXIT_SUCCESS;
}

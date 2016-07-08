#include <stdlib.h>
#include "lib/fconsola.h"

int main(int argc, char **argv) {

	system("clear");
	setvbuf(stdout, NULL, _IONBF, 0);

	//	validar_argumentos(argc); // Se le debe ingresar la ruta del script. Aunque se ejecute por #!

	crearLoggerConsola();

	leerArchivoDeConfiguracion(RUTA_CONFIG_CONSOLA);

	leerScript(argv[1]);

	conectarCon_Nucleo(); // Conexión con Núcleo y espera activa de mensajes.

	esperarMensajesDeNucleo();

	liberarRecursos(); // Libera memoria asignada

	cerrarSocket(fd_nucleo);

	return EXIT_SUCCESS;
}

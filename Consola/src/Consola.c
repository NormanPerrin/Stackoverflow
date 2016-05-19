#include <stdlib.h>
#include "lib/fconsola.h"

int main(int argc, char **argv) {

//	validar_argumentos(argc); // Se le debe ingresar la ruta del script. Aunque se ejecute por #!

	leerArchivoDeConfiguracion(RUTA_CONFIG_CONSOLA); // Abro archivo configuración

	conectarConNucleo(); // Conexión con Núcleo

//	enviar_script(argv[1]); // Envia ruta de script a Núcleo

	esperar_mensajes();

	liberarRecusos(); // Libera memoria asignada

	return EXIT_SUCCESS;
}

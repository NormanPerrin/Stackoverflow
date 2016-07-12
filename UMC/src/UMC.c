#include <stdio.h>
#include <stdlib.h>
#include "lib/fumc.h"

/* - Consola
 * 		- "retardo" (cambia cantidad en ms del retardo)
 * 		- "dump" (reporte por pantalla y archivo de: [tabla páginas, contenido memoria])
 * 		- "flush tlb" (limpia la TLB)
 * 		- "flush memory" (marca todas las páginas de memoria como leídas)
 * - Señales
 * - Ver habilitación/deshabilitación TLB
 * - Verificar si no hay marcos globales (si no hay más marcos libres, tengo 4 marcos por proceso y 3 marcos asignados
 *   a un proceso entonces reemplazo entre esos 3 marcos)
 * */

int main(void) {

	leerArchivoDeConfiguracion(RUTA_CONFIG); // Abro archivo de configuración

	iniciarEstructuras(); // Crea memoria y estructuras de administracións

	conectarConSwap(); // Conexión con Swap

//	crearHilos(); // Creo hilos Servidor y Consola

	liberarRecusos();

	return EXIT_SUCCESS;
}

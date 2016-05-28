#include <stdlib.h>

#include "lib/fTester.h"

int main(int argc, char **argv) {

	validar_argumentos(argc);

	testear(argv[1], argv[2]);

	return EXIT_SUCCESS;
}

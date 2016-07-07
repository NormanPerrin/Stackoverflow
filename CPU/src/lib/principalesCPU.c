#include "principalesCPU.h"

// --LOGGER--
void crearLogger(){
	char * archivoLogCPU = strdup("CPU_LOG.log");
	logger = log_create("CPU_LOG.log", archivoLogCPU, TRUE, LOG_LEVEL_INFO);
	free(archivoLogCPU);
	archivoLogCPU = NULL;
}

void conectarConUMC(){
	fdUMC = nuevoSocket();
	int ret = conectarSocket(fdUMC, config->ipUMC, config->puertoUMC);
	validar_conexion(ret, 1);
	handshake_cliente(fdUMC, "P");
}

void obtenerTamanioDePagina(){
	int * tamPagina = (int*)malloc(INT);
	recibirPorSocket(fdUMC, tamPagina, INT);
	tamanioPagina = *tamPagina; // setea el tamaño de pág. que recibe de UMC
	free(tamPagina);
}

void conectarConNucleo() {
	fdNucleo = nuevoSocket();
	int ret = conectarSocket(fdNucleo, config->ipNucleo, config->puertoNucleo);
	validar_conexion(ret, 1); // Es terminante por ser cliente
	handshake_cliente(fdNucleo, "P");
}

void liberarEstructuras() {
	free(config->ipUMC);
	free(config);

	log_destroy(logger);
	logger = NULL;

	liberarPcbActiva();
	free(infoQuantum);
}

#include "principalesCPU.h"

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

void ejecutarProcesos(){

	int protocolo;
	void * entrada = aplicar_protocolo_recibir(fdNucleo, &protocolo);

		while (entrada != NULL){
			switch (protocolo){
				case PCB:{
					pcbActual = (pcb*)malloc(sizeof(pcb));
					memcpy(pcbActual, entrada, sizeof(pcb));

					ejecutarProcesoActivo(pcbActual);

				break;
			}
				case QUANTUM_MODIFICADO:{
					info_quantum* nuevoQuantum = (info_quantum*) entrada;
					infoQuantum->quantum = nuevoQuantum->quantum;
					infoQuantum->retardoQuantum = nuevoQuantum->retardoQuantum;
					free(nuevoQuantum);
				break;
			}
				default:
					log_error(logger,"Recibí como un mensaje con el protocolo %d no reconocido",protocolo);
			}
			free(entrada);
				entrada = aplicar_protocolo_recibir(fdNucleo, &protocolo);
		}

		cerrarSocket(fdNucleo);
		cerrarSocket(fdUMC);
}

void liberarEstructuras() {
	free(config->ipUMC);
	free(config);

	log_destroy(logger);
	logger = NULL;

	liberarPcb(pcbActual);
	free(infoQuantum);
}

#include "principalesCPU.h"
#define PCBEJECUTANDO pcb

void conectarConUMC(){
	fd_clienteUMC = nuevoSocket();
	int ret = conectarSocket(fd_clienteUMC, config->ipUMC, config->puertoUMC);
	validar_conexion(ret, 1);
	handshake_cliente(fd_clienteUMC, "P");

	int * tamPagina = (int*)malloc(INT);
	recibirPorSocket(fd_clienteUMC, tamPagina, INT);
	tamanioPagina = *tamPagina; // setea el tamaño de pág. que recibe de UMC

}

void conectarConNucleo() {
	fd_clienteNucleo = nuevoSocket();
	int ret = conectarSocket(fd_clienteNucleo, config->ipNucleo, config->puertoNucleo);
	validar_conexion(ret, 1); // Es terminante por ser cliente
	handshake_cliente(fd_clienteNucleo, "P");
	int protocolo;

	void * mensaje = aplicar_protocolo_recibir(fd_clienteNucleo, &protocolo);
	if (protocolo == PCB){
		while(mensaje!=NULL){
				// El CPU obtiene una PCB para ejecutar:
				pcb * pcbEnEjecucion = malloc(sizeof(pcb));
				PCBEJECUTANDO=pcbEnEjecucion;
				memcpy(pcbEnEjecucion, mensaje, sizeof(pcb));
				free(mensaje);

				ejecutarProceso(pcbEnEjecucion);
			}
	}else{
			log_error(logger, "Se esperaba un PCB y no se recibió.");
				//abort();
			}

	cerrarSocket(fd_clienteUMC);
	cerrarSocket(fd_clienteNucleo);
}

void liberarEstructura() {
	free(config->ipUMC);
	free(config);
}

#ifndef LIB_FUNCIONES_H_
#define LIB_FUNCIONES_H_

#include "globales.h"
#include <parser/metadata_program.h>

/**** FUNCIONES SECUNDARIAS ****/
void setearValores_config(t_config * archivoConfig);
void crearLogger();
void actualizarDatosDePcbEjecutada(cpu * unCPU, pcb * pcbEjecutada);
void inicializarIndices(pcb* pcb, t_metadata_program* metaData);
int asignarPid(t_list * procesos);
pcb* buscarProcesoPorPid(int pid, int* index);
pcb* crearPcb(string programa);
void salvarProcesoEnCPU(int cpuId);
void liberarPcb(pcb * pcb);
void planificarProceso();
void finalizarPrograma(int pid);
void limpiarListasYColas();
void liberarConsola(consola * consola);
void liberarCPU(cpu * cpu);
void limpiarArchivoConfig();
void manejarES();
void notificarCambioDelQuantumACPU();
void detectarCambiosEnArchivoConfig();
void enviarNuevoQuantum(cpu * unCpu);
int solicitarSegmentosAUMC(pcb * nuevoPcb, string programa);
pcb* copiarPcb(pcb* proceso);
void encolarPcbAListos(pcb* proceso);

#endif /* LIB_FUNCIONES_H_ */

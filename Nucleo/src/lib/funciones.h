#ifndef LIB_FUNCIONES_H_
#define LIB_FUNCIONES_H_

#include "globales.h"

/**** FUNCIONES SECUNDARIAS ****/
int setearValoresDeConfig(t_config * archivoConfig);
void actualizarPcbEjecutada(cpu * unCPU, pcb * pcbEjecutada, int estado);
void inicializarIndices(pcb* pcb, t_metadata_program* metaData);
int asignarPid(t_list * procesos);
pcb* buscarProcesoPorPid(int pid, int* index);
pcb* crearPcb(char* programa);
void salvarProcesoEnCPU(int cpuId);
void planificarProceso();
void finalizarPrograma(int pid, int index);
void limpiarColecciones();
void liberarSemaforo(t_semaforo * sem);
void liberarVarCompartida(var_compartida * var);
void liberarConsola(consola * consola);
void liberarCPU(cpu * cpu);
void limpiarArchivoConfig();
int solicitarSegmentosAUMC(pcb * nuevoPcb, char* programa);
pcb* copiarPcb(pcb* proceso);
void encolarPcbAListos(pcb* proceso);
int pcbListIndex(int pid);
var_compartida* crearVariableCompartida(char* nombre, int valorInicial);
void registrarSemaforo(char* name, int value);
void registrarVariableCompartida(char* name, int value);

// IO y Semaforos:
// IO:
void bloquearProcesoPorIO(hiloIO* dispositivoIO, proceso_bloqueadoIO* unPcb);
void realizarEntradaSalida(pcb* procesoEjecutando, pedidoIO* datos);
void* entradaSalidaThread(void* dataHilo);
hiloIO* crearHiloIO(int index);
proceso_bloqueadoIO* esperarPorProceso(dataDispositivo* datos);

// TAD SEmáforo:
t_semaforo* semaforo_create(char*nombre, int valor);
void semaforo_signal(t_semaforo* semaforo);
int semaforo_wait(t_semaforo* semaforo);
void semaforo_blockProcess(t_queue* colaBloqueados, pcb* procesoEjecutando);

#endif /* LIB_FUNCIONES_H_ */

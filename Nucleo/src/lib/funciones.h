#ifndef LIB_FUNCIONES_H_
#define LIB_FUNCIONES_H_

#include "globales.h"

// Configuración y setting:
void iniciarEscuchaDeInotify();
int setearValoresDeConfig(t_config * archivoConfig);
t_semaforo* semaforo_create(char*nombre, int valor);
void registrarSemaforo(char* name, int value);
var_compartida* crearVariableCompartida(char* nombre, int valorInicial);
// Lanzar hilos IO:
void registrarVariableCompartida(char* name, int value);
hiloIO* crearHiloIO(int index);
int validar_cliente();
int validar_servidor();
proceso_bloqueadoIO* esperarPorProcesoIO(dataDispositivo* datos);
void encolarPcbAListos(pcb* proceso);
void* entradaSalidaThread(void* dataHilo);
// Función select - Planificación:
int obtenerSocketMaximoInicial();
void planificarProceso();
pcb* buscarProcesoPorPid(int pid);
int asignarPid();
int solicitarSegmentosAUMC(pcb * nuevoPcb, char* programa);
pcb* crearPcb(char* programa);
void aceptarConexionEntranteDeConsola(); // --> planificarProceso
void aceptarConexionEntranteDeCPU(); // --> planificarProceso
void atenderCambiosEnArchivoConfig(int* socketMaximo);
void salvarProcesoEnCPU(int cpuId); // --> planificarProceso
int envioSenialCPU(int id_cpu);
void finalizarPrograma(int pid, int index);
int pcbListIndex(int pid);
void realizarEntradaSalida(pcb* procesoEjecutando, pedidoIO* datos);
void semaforo_signal(t_semaforo* semaforo);
int semaforo_wait(t_semaforo* semaforo);
void semaforo_blockProcess(t_queue* colaBloqueados, pcb* proceso);
void recorrerListaCPUsYAtenderNuevosMensajes();
// Liberar recursos:
void liberarCPU(cpu * cpu);
void liberarConsola(consola * consola);
void liberarSemaforo(t_semaforo * sem);
void liberarVarCompartida(var_compartida * var);
void limpiarColecciones();
void limpiarArchivoConfig();

extern bool seDesconectoUMC;

#endif /* LIB_FUNCIONES_H_ */

#ifndef LIB_FCPU_H_
#define LIB_FCPU_H_

	#include <utilidades/general.h>
	#include <commons/config.h>
	#include <commons/log.h>

	#define PACKAGESIZE 1024 // Size máximo de paquete para sockets
	#define RUTA_CONFIG_CPU "configCPU.txt"

	// Estructuras
	typedef struct {
		int puertoNucleo; // Puerto donde se encuentra escuchando el proceso Núcleo
		char *ipNucleo; // IP del proceso Núcleo
		int puertoUMC;
		char *ipUMC;
	} t_configuracion;

	// Cabeceras
	void conectarConNucleo();
	void conectarConUMC();
	void setearValores_config(t_config * archivoConfig);
	void liberarEstructura(); // Libera la memoria reservada en setear config
	int validar_servidor(char *id); // Valida si la conexión es UMC o Nucleo
	int validar_cliente(char *id); // para que no joda con error

#endif /* LIB_FCPU_H_ */

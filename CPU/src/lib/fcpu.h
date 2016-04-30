#ifndef LIB_FCPU_H_
#define LIB_FCPU_H_

#include <utilidades/sockets.h>
#include <utilidades/general.h>
#include <commons/config.h>
#include <commons/log.h>
#define PACKAGESIZE 1024 // Size máximo de paquete para sockets

// Variables globales
int puertoNucleo; // Puerto donde se encuentra escuchando el proceso Núcleo
char *ipNucleo; // IP del proceso Núcleo
int puertoUMC;
char *ipUMC;

// Estructuras

// Cabeceras
void conectarConNucleo();
void conectarConUMC();
void setearValores_config(t_config * archivoConfig);
void testLecturaArchivoDeConfiguracion();

#endif /* LIB_FCPU_H_ */

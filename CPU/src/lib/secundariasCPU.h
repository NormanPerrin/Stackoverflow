#ifndef LIB_SECUNDARIASCPU_H_
#define LIB_SECUNDARIASCPU_H_

#include "primitivasAnSISOP.h"

/** PROTOTIPO FUNCIONES SECUNDARIAS **/
void setearValores_config(t_config * archivoConfig);
int validar_servidor(char *id); // Valida si la conexión es UMC o Nucleo
int validar_cliente(char *id); // para que no joda con error
void recibirYvalidarEstadoDelPedidoAUMC();
void liberarPcbActiva();

#endif /* LIB_SECUNDARIASCPU_H_ */

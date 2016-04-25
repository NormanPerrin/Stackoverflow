#include "fumc.h"

t_configuracion* abrirArchivoConfig(char *ruta) {

	t_config *config = config_create(ruta);

	t_configuracion *ret = (t_configuracion*)reservarMemoria(sizeof(t_configuracion));

	ret->puerto = config_get_int_value(config, "PUERTO");
	ret->ip_swap = reservarMemoria(CHAR*16);
	ret->ip_swap = config_get_string_value(config, "IP_SWAP");
	ret->puerto_swap = config_get_int_value(config, "PUERTO_SWAP");
	ret->marcos = config_get_int_value(config, "MARCOS");
	ret->marco_size = config_get_int_value(config, "MARCO_SIZE");
	ret->marco_x_proceso = config_get_int_value(config, "MARCO_X_PROC");
	ret->entradas_tlb = config_get_int_value(config, "ENTRADAS_TLB");
	ret->retardo = config_get_int_value(config, "RETARDO");

	return ret;
}


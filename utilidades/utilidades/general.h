#ifndef UTILIDADES_GENERAL_H_
#define UTILIDADES_GENERAL_H_

	#define FALSE 0
	#define TRUE 1
	#define INT (sizeof(int))
	#define CHAR (sizeof(char))

	/**
	* @NAME: reservarMemoria
	* @DESC: Crea dinámicamente y devuelve un puntero a void
	* @PARAMS:
	*		size - Tamaño en bytes del bit array
	*/
	void* reservarMemoria(int size);


#endif

#include "fnucleo.h"

// Funciones

void setearValores_config(t_config * archivoConfig){
	puertoPrograma = config_get_int_value(archivoConfig, "PUERTO_PROG");
	puertoCPU = config_get_int_value(archivoConfig, "PUERTO_CPU");
	puertoUMC = config_get_int_value(archivoConfig, "PUERTO_UMC");
	ipUMC = strdup(config_get_string_value(archivoConfig, "IP_UMC"));
	quantum = config_get_int_value(archivoConfig, "QUANTUM");
	retardoQuantum = config_get_int_value(archivoConfig, "QUANTUM_SLEEP");
	pasarCadenasArray(semaforosID, config_get_array_value(archivoConfig, "SEM_IDS"));
	pasarCadenasArray(ioID, config_get_array_value(archivoConfig, "IO_IDS"));
	pasarCadenasArray(variablesCompartidas, config_get_array_value(archivoConfig, "SHARED_VARS"));
	pasarEnterosArray(semaforosValInicial, config_get_array_value(archivoConfig, "SEM_INIT"));
	pasarEnterosArray(retardosIO, config_get_array_value(archivoConfig, "IO_SLEEP"));

}

void escucharACPU(){
	int fd_escuchaCPU, fd_nuevoCPU;

			fd_escuchaCPU = nuevoSocket();
			asociarSocket(fd_escuchaCPU, puertoCPU);
			escucharSocket(fd_escuchaCPU, CONEXIONES_PERMITIDAS);
			fd_nuevoCPU= aceptarConexionSocket(fd_escuchaCPU);

			printf("CPU conectado. Esperando mensajes\n");
			/*escucharSocket(fd_escuchaCPU, CONEXIONES_PERMITIDAS);*/// Ponemos a esuchar de nuevo al socket escucha

			char package[PACKAGESIZE];
				int status = 1;		// Estructura que manjea el status de los recieve.
				// Vamos a ESPERAR que nos manden los paquetes, y los imprimos por pantalla
				while (status != 0){
					status = recibirPorSocket(fd_nuevoCPU, (void*) package, PACKAGESIZE);
					if (status != 0) printf("%s", package);

				}
				close(fd_nuevoCPU);
					close(fd_escuchaCPU);

} // Soy servidor, espero mensajes de algún CPU

void escucharAConsola(){
	int fd_escuchaConsola, fd_nuevaConsola;

		fd_escuchaConsola = nuevoSocket();
		asociarSocket(fd_escuchaConsola, puertoPrograma);
		escucharSocket(fd_escuchaConsola, CONEXIONES_PERMITIDAS);
		fd_nuevaConsola= aceptarConexionSocket(fd_escuchaConsola);

		printf("Consola conectada. Esperando mensajes\n");
		/*escucharSocket(fd_escuchaConsola, CONEXIONES_PERMITIDAS);*/// Ponemos a esuchar de nuevo al socket escucha

		char package[PACKAGESIZE];
			int status = 1;		// Estructura que manjea el status de los recieve.
			// Vamos a ESPERAR que nos manden los paquetes, y los imprimimos por pantalla
			while (status != 0){
				status = recibirPorSocket(fd_nuevaConsola, (void*) package, PACKAGESIZE);
				if (status != 0) printf("%s", package);

			}
			close(fd_nuevaConsola);
				close(fd_escuchaConsola);
} // Soy servidor, espero mensajes de alguna Consola

void conectarConUMC(){
	int fd_serverUMC;

	fd_serverUMC = nuevoSocket();
		asociarSocket(fd_serverUMC, puertoUMC);
		conectarSocket(fd_serverUMC, ipUMC, puertoUMC);
		// Creo un paquete (string) de size PACKAGESIZE, que le enviaré a la UMC
		int enviar = 1;
			char message[PACKAGESIZE];

			printf("Conectado a la UMC. Ya se puede enviar mensajes. Escriba 'exit' para salir\n");

			while(enviar){
				fgets(message, PACKAGESIZE, stdin);	// Lee una línea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE
				if (!strcmp(message,"exit\n")) enviar = 0; // Chequeo que no se quiera salir
				if (enviar) enviarPorSocket(fd_serverUMC, message, strlen(message) + 1); // Sólo envío si no quiere salir
			}
			close(fd_serverUMC);
	} // Soy cliente de la UMC, es  decir, soy el que inicia la conexión con ella

void crearLogger(){
	char * archivoLogNucleo = strdup("NUCLEO_LOG.log");
	logger = log_create("NUCLEO_LOG.log",archivoLogNucleo,false,LOG_LEVEL_INFO);
	free(archivoLogNucleo);
	archivoLogNucleo = NULL;
}

void testLecturaArchivoDeConfiguracion(){
	printf("Puerto de Programa: %d\n",puertoPrograma);
	printf("Puerto de CPU: %d\n",puertoCPU);
	printf("Quantum de Round Robin: %d\n",quantum);
	printf("Retardo de Quantum: %d\n",retardoQuantum);
	printf("Semaforos: "); imprimirCadenas(semaforosID);
	printf("Cantidad de Semaforos: "); imprimirNumeros(semaforosValInicial);
	printf("Dispositivos de I/O: "); imprimirCadenas(ioID);
	printf("Retardos de I/O: "); imprimirNumeros(retardosIO);
	printf("Variables compartidas: "); imprimirCadenas(variablesCompartidas);
}

// --Funciones MUY auxiliares

void imprimirCadenas(char** cadenas){
	int i;
	for(i=0; i<NELEMS(cadenas);i++){
		printf("%s, ", cadenas[i]);
	}
}

void imprimirNumeros(int* numeros){
	int i;
		for(i=0; i<NELEMS(numeros);i++){
			printf("%d, ", numeros[i]);
		}
}

void pasarCadenasArray(char** cadenas, char** variablesConfig){
	int cantidadCadenas, i;

	cantidadCadenas = NELEMS(variablesConfig);

	for(i=0; i<(cantidadCadenas-1); i++){
		cadenas[i]= strdup(variablesConfig[i]);
		}
	}

void pasarEnterosArray(int* numeros, char** variablesConfig){
	int cantidadNumeros, i;

	cantidadNumeros = NELEMS(variablesConfig);

	for(i=0; i<(cantidadNumeros-1); i++){
		numeros[i] = atoi(variablesConfig[i]);
	}
}

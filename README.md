# elestac
Sistema que simula el funcionamiento de un SO mediante la ejecución de scripts

## Instalar ambiente de trabajo:
1. `git clone http://github.com/sisoputnfrba/so-commons-library.git` e instalar `sudo make install`
3. `git clone http://github.com/sisoputnfrba/ansisop-parser.git` e instalar `sudo make install`
4. `git clone http://github.com/NormanPerrin/elestac.git` e instalar
5. Para instalar primero se hace `sudo make all` de la shared library utilidades
6. Luego se hace `sudo make all` de cada módulo
7. Ahora hay que linkear la librería utilidades con los módulos a ejecutar con el comando export. Se puede hacer directamente o modificando el archivo .bashrc. Si se hace modificando el archivo no hay necesidad de ejecutar el comando cada vez que se abra una nueva consola

## Links importantes:
- [Enunciado](https://sisoputnfrba.gitbooks.io/elestac-tp-1c2016)
- [Scripts para ejecutar](https://github.com/sisoputnfrba/scripts-ansisop)
- [Pruebas para ejecutar](http://faq.utn.so/elestac-pruebas)

## Instalación de Scripts AnSISOP:
- Se pueden ejecutar como argumento desde Consola o directamente
- Para ejecutar directamente hay que seguir los siguientes pasos:
1. Cambiamos el hashbang (#!) a la ruta donde está el ejecutable Consola (home/.../Consola/Debug/Consola)
2. Lo hacemos ejecutable: `chmod +x script.ansisop`

## Forma de levantar el sistema
1. Swap
2. UMC
3. Nucleo
4. n instancias de CPU
5. Ejecutamos un script en la carpeta de Consola

## Aclaraciones
- Un script se puede inicializar sin CPU conectada, pero no va a ejecutar instrucciones
- Se puede finalizar un script de forma abortiva o esperando a que termine (si tiene fin)
- Se deben poner los archivos de configuración en las carpetas Debug ya que la ruta que leen es relativa con el nombre "configUMC.txt" para el caso de UMC
- Se pueden ver los resultados de la ejecución en tiempo real por la terminal o en los logs que generan
- Puede que haya problemas al abortar una CPU en ejecución

## Comandos útiles:
- `export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:directorio/utilidades/Debug` linkear librería
- `watch -s 1 hexdump -c nombArchi.data` mapear el archivo físico

## Mandar señal SIGUSR1 al proceso CPU:
1. Averigüar el PID desde una nueva consola: `~$ pidof CPU`
2. Usamos el PID devuelto por el comando anterior para terminar el proceso: `~$ kill -USR1 pid`

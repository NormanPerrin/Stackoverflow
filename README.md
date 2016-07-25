## Instalar ambiente de trabajo:
1. `git clone http://github.com/sisoputnfrba/tp-2016-1c-Cazadores-de-cucos.git`
2. Abrir eclipse
3. File - New - Import - General - Existing...
4. Browse - Seleccionan la carpeta "tp-2016-1c-Cazadores-de-cucos" y OK
5. Ahora hay que hacer que linken las librerías
  1. Ir con consola a la carpeta de commons de nuestro repositorio
  2. Escribimos en consola: `sudo make install`
  3. Repetimos para el parser
6. Agregamos el path para la shared library para eclipse
  1. Run - Run Configurations - Environment - New
  2. Name: LD_LIBRARY_PATH
  3. Value: ${workspace_loc}/utilidades/Debug (Pueden ir a Variables, buscar work y agregarlo. Cada uno tiene valores diferentes)
  4. Aply
7. Para linkear la shared para consola
  1. Abren la shell. Van a home con "~"
  2. Escriben: vim .bashrc
  3. Apretan la "i" y van abajo de todo
  4. Escriben: `export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/Escritorio/tp-2016../utilidades/Debug` (cada uno tiene valores diferentes)
  5. Aprietan Ctrl + C y escriben ":wq" y enter
  6. Para verificar que les funcionó pueden ir al directorio del ejecutable y escribir: `ldd Swap` y les debería aparecer la shared con una dirección
  7. Si no quieren hacer todo esto pueden optar por usar el comando `export` cuando lo necesiten estando en el directorio del ejecutable. Pero cada vez que abran una consola nueva lo van a tener que escribir de nuevo
  
## Comandos Github:
- Link: http://blog.desdelinux.net/guia-rapida-para-utilizar-github/

## Debuguear:
- [Tutorial de UTN](https://youtu.be/XsefDXRfA9k)
- F6: siguiente instrucción
- F5: meterse en instrucción
- Para debuggear hilos usar breakpoints

## Links importantes:
- [Enunciado]()
- [Tutorial básico GitHub](https://youtu.be/cEGIFZDyszA?list=PL6gx4Cwl9DGAKWClAD_iKpNC0bGHxGhcx)
- [Direccionario UTN](http://faq.utn.so/)
- [Git Difftool](https://youtu.be/iCGrKFH2oeo)

## Comandos Consola Linux:
- `less config.txt` Muestra contenido del archivo. Se sale con q
- `vim config.txt` Te deja modificar el archivo
	- Para insertar: apretar "i" y moverse con flechas
	- Para guardar: apretar Esc. ":" y va a aparecer una especie de consola abajo, escribir "wq" (write)(quit)
- `cd dir` Para cambiar de directorio (chage directory)
- `ls` Muestra los directorios disponibles
- `export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:directorio/utilidades/Debug`
- `mkdir dirName` Crea un directorio (make directory)
- `ln -s directorio nombre` Crea un acceso directo simbólico a directorio llamado nombre
- `pwd` Muestra el directorio completo del lugar actual
- `rm file` Elimina archivo (remove)
- `rm -fr dir` Eliminar todo el directorio dir
- `make all` Compilar módulo

## Pasos para el Deploy:
- En caso de instalar con pendrive: descompilar con  `tar -xvf commons_ultimo.tar`
- En caso de instalar con internet: `git clone http://github.com/sisoputnfrba/tp-2016-1c-Cazadores-de-cucos.git`
- Abrir terminal agregar export al bashrc sino export a cada módulo
- Ir a la carpeta cada módulo y ejecutar el makefile (`make all`)
- Mover configs de cada módulo a la carpeta Debug correspondiente (`cp -f config.txt modulo/Debug/config.txt`)

## Instalación de Scripts AnSISOP:

- Revisar si los scripts ya tienen permisos de ejecución. Para ello, vamos con la terminal a la ubicación de los scripts y utilizamos el comando `ls`, si los nombres de los scripts aparecen resaltados en verde es que ya están bien.
- Sino aparecen en verde, parados en la terminal en la misma ubicación escribimos `sudo chmod 775 facil.ansisop` para cada uno de los scripts. Luego, verificamos nuevamente que sí aparezcan en verde con `ls`.

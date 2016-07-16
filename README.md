## Instalar ambiente de trabajo
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

## Vamos a trabajar todos sobre la rama 'master'. Comandos necesarios:
1. *Indicar la direcicón de mi proyecto:* `cd /home/utnso/...` (donde lo tengan guardado)
2. *Averiguar el estado de mi proyecto:* `git status`
3. *Bajar del repo la última versión del proyecto:* `git pull` (siempre antes de un push)
4. *Agregar (todos) los cambios realizados:* `git add .`
5. *Agregar solo un cambio realizado:* `git add nombre_del_archivo`
5. *Hacer el commit correspondiente:* `git commit -m "los cambios que hice"`
6. *Actualizar la versión existente en Github (subir lo que hice):* `git push`

Más comandos acá (branches, etc.): http://blog.desdelinux.net/guia-rapida-para-utilizar-github/

## Para implementar/usar alguna de las common libraries en un .c/.h debe incluirse así:

- Logging: `#include <commons/log.h>`
- Manipulación de Strings: `<commons/string.h>`
- Manipulación de archivos de configuración: `<commons/config.h>`
- Manejo/Funciones de fechas: `<commons/temporal.h>`
- Manejo de array de bits: `<commons/bitarray.h>`
- Información de procesos: `<commons/process.h>`
- Manejo simple de archivos de texto: `<commons/txt.h>`
Conjunto de elementos:
- List: `#include <commons/collections/list.h>`
- Dictionary: `#include <commons/collections/dictionary.h>`
- Queue: `#include <commons/collections/queue.h>`

## Debuguear
- [Tutorial de UTN](https://youtu.be/XsefDXRfA9k)
- F6: siguiente instrucción
- F5: meterse en instrucción
- Para debuggear hilos usar breakpoints

## Links importantes
- [Enunciado]()
- [Tutorial básico GitHub](https://youtu.be/cEGIFZDyszA?list=PL6gx4Cwl9DGAKWClAD_iKpNC0bGHxGhcx)
- [Direccionario UTN](http://faq.utn.so/)
- [Git Difftool](https://youtu.be/iCGrKFH2oeo)

## Comandos
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

## Pasos para deploy
- En caso de instalar con pendrive: descompilar con  `tar -xvf commons_ultimo.tar`
- En caso de instalar con internet: `git clone http://github.com/sisoputnfrba/tp-2016-1c-Cazadores-de-cucos.git`
- Abrir terminal agregar export al bashrc sino export a cada módulo
- Ir a la carpeta cada módulo y ejecutar el makefile (`make all`)
- Mover configs de cada módulo a la carpeta Debug correspondiente (`cp -f config.txt modulo/Debug/config.txt`)

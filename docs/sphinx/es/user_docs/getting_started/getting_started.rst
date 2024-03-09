.. _comenzando:

##########
Comenzando
##########

Compilando mrv2
---------------

Por favor refiérase a la documentación de github en:

https://github.com/ggarra13/mrv2


Instalando mrv2
---------------

- En macOS usted lo instala abriendo el archivo .dmg, y llevando el ícono de mrv2 icon al directorio de Aplicaciones. Si ya hay una versión de mrv2, le recomendamos que la sobreescriba. La aplicación de macOS actualmente no está notarizada, por lo que cuando la ejecute macOS le avisará que el archivo no es seguro porque fue bajado de la internet. Para evitar eso, necesita abrir el Finder, ir al directorio de Aplicaciones y presionar CTRL + boton izquierdo del ratón sobre la aplicación mrv2. Esta acción traerá la misma advertencia, pero esta vez tendrá un botón que le permitirá abrirlo. Necesitará hacer esto sólo una vez.

- Windows y Chrome, como macOS, también te protejen de instalar archivos de Internet. Cuando lo baja con Chrome puede que le avise que no es un archivo usual para ser bajado. Asegúrese de cliquear en el menu de la flecha derecha arriba para seleccionar "Grabar de todas formas". No puede ejecutar un .exe directo de Chrome. Tendra que abrir la carpeta contenedora o usar Windows Explorer e ir al directorio de Descargas. Luego deberá correrlo desde ahí. Windows le abrirá un mensaje Azul avisándole que SmartScreen previno el arranque de una aplicación desconocidas y que puede poner a su PC en peligro. Cliquée en Más Informacion y un botón que dice Correr de todas formas o similar aparecerá. Cliquee en él y siga las intruciones usuales del instalador de Windows.

- En Linux, para instalar los paquetes .rpm o .deb requiere que su usuario tenga permisos de administrador (sudo).

En Debian (Ubuntu, etc), lo instalaría con::

  sudo dpkg -i mrv2-v1.0.9-amd64.tar.gz
  
En Red Hat (Rocky Linux, etc), lo instalaría con::

  sudo rpm -i mrv2-v1.0.9-amd64.tar.gz

Una vez que lo instala, puede correr mrv2 tipeando::

  mrv2

en la terminal, ya que un enlace simbólico al ejecutable se agrega en /usr/bin. Los instaladores asociarán la extensión de archivos y instalarán un ícono para arrancarlo fácilmente desde el Escritorio para el usuario que lo instaló.

Para correr mrv2 con el ícono, necesita seleccionarlo y usar el botón derecho del ratón para abrir el menú y elegir Permitir Lanzar.

Si no tiene permisos de super usuario en su organización, debería bajar el archivo .tar.gz y descomprimirlo con::

  tar -xf mrv2-v1.0.9-amd64.tar.gz
  
Eso creará una carpeta en el directorio desde donde lo descomprimió. Podrá correr mrv2 usando el script de bash::

  mrv2.sh

que se encuentra en el subdirectorio bin/.

Wayland support
---------------

La distribución binaria de mrv2, que es compilada en Rocky Linux 8.9, tiene sporte para Wayland on Linux, pero puede tener problemas en arrancar en nuevas distros.  Se recomienda que lo compile del código fuente.


Lanzando mrv2
-------------

Mientras que en Linux, puede lanzar mrv2 de la terminal o del ícono, en Windows y macOS por defecto usará el ícono provisto en el lugar de instalación del ejecutable.

.. image:: ../images/interface-01.png

Cargando Medios (Drag and Drop)
-------------------------------

Una forma facil de cargar media es usar archivos o carpetas drag-and-drop en la ventana principal desde el buscador del sistema (Finder, Explorer, Nautilus, etc). Si usted deja una carpeta, el directorio será recursivamente escaneado y todos los clips serán agregados.

Cargando Medios (Buscador de mrv2)
----------------------------------

Usando **Archivo->Abrir Película o Sequencia**, un buscador de archivos se abrirá.  Por defecto, el buscador de archivos es el propio de mrv2.  Sin embargo, en **Ventana->Preferencias->Buscador de Archivos** puede seleccionar "Usar Buscador de Archivo Nativo" que usará el buscador de archivo nativo a su plataforma.

.. note::
   En macOS, el buscador propio de mrv2 puede que no pueda abrir los directorios
   protegidos del sistema como Escritorio, Descargas, Documentos, etc. ya que
   el OS no se lo permite debido a que mrv2 no está registrado.
   Tampoco podrá usar el Finder con el boton derecho del ratón y la opción
   Abrir con mrv2.

Cargando Medios (Menu Reciente)
-------------------------------

Si quiere cargar hasta 10 clips que recientemente los cerró o los cargó previamente, puede hacerlo desde **Archivo->Reciente**.


Cargando Medios (Línea de comandos)
-----------------------------------

Clips pueden ser cargados usando la línea de comando de una ventana de la terminal que es conveniente y poderosa forma para usuarios familiazados con ella.

mrv2 supporta varios modos para cargar secuencias y películas.  Puede mezclar varios modos como requiera hasta tres clips::

    mrv2 /path/to/test.mov /path/to/frames.0001.exr /path/to/edit.otio

.. note::
     Archivos de películas serán reproducidos a su velocidad 'natural'.  En otras palabras, mrv2 respeta la velocidad codificada de FPS del archivo dado.
     
.. note::
    Secuencias de imágenes (una serie de archivos JPEG o TGA) por defecto usan 24 FPS (puede ajustarlo en **Window->Preferencias->Reproducir**). La velocidad de DPX and EXRs es tomada de los metadatos si está disponible.

Mirando Medios
--------------

El primer item de medios que es agregado a mrv2 será visible y podrá empezar a verlo entero o en bucle.  Para mirar a otros clips, puede mostrar el Panel de Archivos (F4 por defecto).  Con él podrá cliquear en la película que quiera ver.  

Cuando se carga un clip, el comportamiento por defecto del playback puede ser seteado en **Ventana->Preferencias->Reproducir** y cliquear en Auto Reproducir.

.. _comenzando:

##########
Comenzando
##########

Compilando mrv2
---------------

Por favor refiérase a la documentación de github en:

http::https://github.com/ggarra13/mrv2


Instalando mrv2
---------------

- En macOS usted lo instala abriendo el archivo .dmg, y llevando el ícono de mrv2 icon al directorio de Aplicaciones. Si ya hay una versión de mrv2, le recomendamos que la sobreescriba. La aplicación de macOS actualmente no está notarizada, por lo que cuando la ejecute macOS le avisará que el archivo no es seguro porque fue bajado de la internet. Para evitar eso, necesita abrir el Finder, ir al directorio de Aplicaciones y presionar CTRL + boton izquierdo del ratón sobre la aplicación mrv2. Esta acción traerá la misma advertencia, pero esta vez tendrá un botón que le permitirá abrirlo. Necesitará hacer esto sólo una vez.

- Windows y Chrome, como macOS, también te protejen de instalar archivos de Internet. Cuando lo baja con Chrome puede que le avise que no es un archivo usual para ser bajado. Asegúrese de cliquear en el menu de la flecha derecha arriba para seleccionar "Grabar de todas formas". No puede ejecutar un .exe directo de Chrome. Tendra que abrir la carpeta contenedora o usar Windows Explorer e ir al directorio de Descargas. Luego deberá correrlo desde ahí. Windows le abrirá un mensaje Azul avisándole que SmartScreen previno el arranque de una aplicación desconocidas y que puede poner a su PC en peligro. Cliquée en Más Informacion y un botón que dice Correr de todas formas o similar aparecerá. Cliquee en él y siga las intruciones usuales del instalador de Windows.

- On Linux, in order to install the .rpm or .deb packages requires your user to have sudo permissions.

On Debian (Ubuntu, etc) systems, you would install with::

  sudo dpkg -i mrv2-v0.7.0-amd64.tar.gz
  
On Red Hat (Rocky Linux, etc), you would install it with::

  sudo rpm -i mrv2-v0.7.0-amd64.tar.gz

Once you install it, you can run mrv2 by just typing mrv2 in the shell, as a symlink to the executable is placed in /usr/bin. The installers will also associate file extensions and install an icon for easy starting up in the Desktop icon of the user that installed it. For running mrv2 with the icon, you need to select it and use the right mouse button to open the menu and choose Allow Launch.

If you lack sudo permissions in your organization, you should download the .tar.gz file and you can uncompress it with::

  tar -xf mrv2-v0.7.0-amd64.tar.gz
  
That will create a folder in the direcory you uncompress it from. You can then run mrv2 by using the mrv2.sh shell script in the bin/ subdirectory.


Launching mrv2
--------------

To launch an empty mrv2 "Session" type ``mrv2`` on the Linux command line and press enter.

You can drag-and-drop files into the main window from Ivy Browser or the file system, or use the "Load" layout to add Shotgun playlists.

You can also launch mrv2 with one or more media files on the command line.

.. image:: ../images/interface-01.png

Loading Media (Drag and Drop)
-----------------------------

An easy way to load media is to drag-and-drop files or folders into the main window from the file system. If you drop a folder, the directory will be recursively searched for media files and they will all be added.

Loading Media (filesystem browser)
----------------------------------

Using File->Open Movie or Sequence, a File requester will be opened.  By default the file requester is mrv2's custom file requester.  However, in the Window->Preferences->File Requester you can select Use Native File Chooser which will use the native file chooser for your platform.

Loading Media (Recent menu)
---------------------------

If you want to load up to 10 clips that you recently closed or previously loaded, you can do so from File->Recent.


Loading Media (command line)
----------------------------

Media can be loaded using the mrv2 command line from a terminal window which will be convenient and powerful for users familiar with shell syntax. By default, if mrv2 is already running, files will be added to the existing session instead of starting a new session. If you want to launch a new session, use the -n flag.

mrv2 supports various modes for loading sequences and movies.  You can mix different modes as required::

    mrv2 /path/to/test.mov /path/to/\*.jpg /path/to/frames.0001.exr

.. note::
     Movie files will be played back at their 'natural' frame rate, in other words mrv2 respects the encoded frame rate of the given file.
.. note::

    Image sequences (e.g. a series of JPEG or EXR files) default to 24fps (you can adjust this in Window->Preferences).

Viewing Media
-------------

The first media item that is added to mrv2 will be made visible and you can start playing through / looping. To look at other media you can bring the Files Panel (F4 by default).  With it you can click on the file you want to see.  

When loading a clip, the default behavior of playback can be set in the Window->Preferences->Playback and clicking on Auto-Playback.

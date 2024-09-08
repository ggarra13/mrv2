####################
Preguntas Frecuentes
####################

Todas las Plataformas
=====================

- Mi reproducción es lenta.  ¿Cómo puedo mejorarla?

  Chequée que tengas:
  
    * Ventana->Preferencias

      - OpenGL
	
	Precisión de Color seteado a Automático (preferido) o Rápido.

      - Miniatures

	Vista Previa sobre la Línea de Tiempo apagado.

      - Reproducción

	Auto Ocultar Barra de Pixel checked
	o optionalmente ocular la barra de pixel en la Interfaz o
	establecer modo de Presentación.

    * Panel->Settings
      
      - Cache Gigabytes

	Seteado a 4GB por lo menos (por defecto es la mitad de tu memoria)
  
Linux
=====

Todos los protocolos de display
-------------------------------

- La reproducción de las capturas de pantalla no se reproducen a su velocidad
  correcta.

  Lamentablemente, esta es una limitación de mrv2.  Capturas de vídeo de
  pantalla como las de Ubuntu pueden depender de Variable Frame Rates (VFR), ó
  Velocidad de Reproducción Variable.  Esto significa que la velocidad cambia
  de cuadro a cuadro en el vídeo.
  mrv2 necesita una velocidad constante durante todo el vídeo para su
  reproducción, búsqueda y mostrar la línea de tiempo.

X11
---

- Veo un "desgarro" del vídeo durante la reproducción.  ¿Cómo lo arreglo?

  Esto es probable una misconfiguración de su tarjeta gráfica.
  Para tarjetas gráficas de NVidia, puede arreglarlo así:
  
    * Edite /etc/X11/xorg.conf con permisos de superusuario (sudo).

    * Vaya a la sección "Device" de su tarjeta NVIDIA.

    * Agregue o modique::
	
	Option "TripleBuffer" "True"
	Option "SwapInterval" "1"
	
Wayland
-------
	
- Mi reproducción es lenta.  ¿Cómo puedo mejorarla?
  
  Chequée que tengas:
  
    * Ayuda->Acerca

      - Vaya a la pestaña HW.

	Chequée la información de GPU y asegúrese que Vendedor/Renderer no sea
	Mesa, pero el apropiado para su tarjeta gráfica.  Por ejemplo:
	
	Vendedor:   NVIDIA Corporation
	Renderer:   NVIDIA GeForce RTX 3080/PCIe/SSE2
	Versión:    4.6.0 NVIDIA 535.171.04

	Max. Tamaño Textura:32768 x 32768

	Si está usando Mesa, puede que necesite configurar XWayland/Wayland
	correctamente para su OS y tarjeta gráfica.
	O usar mrv2 no en Wayland pero en X11/Xorg.

	En Ubuntu 22.04.4 LTS, deberás instalar los drivers privativos de
	NVidia 535 por lo menos y hacer::

	  sudo apt install libnvidia-egl-wayland1

	Note que el EGL de NVidia bajo Wayland en Ubuntu 22.04.4 LTS Jammy
	parece tener un error y puede resultar en detenimiento aleatorio de la
	reproducción hasta que mueva el ratón.
	Para arreglarlo, puede instalar el paquete .deb de Ubuntu Noble::
	  
	  https://www.ubuntuupdates.org/package/core/noble/main/base/libnvidia-egl-wayland1
	
	Alternativamente, use XWayland o loggese en X11.  Para usar
	XWayland, setée::

	  export FLTK_BACKEND=x11
	  
- Tengo avisos (warnings) cuando ejecuto el mrv2 pre-compilado en la consola
  bajo Wayland en una distro moderna como Ubuntu 22.04.4 LTS::

    (mrv2:6869): GdkPixbuf-WARNING **: 09:23:50.243: Cannot open pixbuf loader module file '/usr/lib64/gdk-pixbuf-2.0/2.10.0/loaders.cache': No such file or directory

    This likely means that your installation is broken.
    Try running the command
    gdk-pixbuf-query-loaders > /usr/lib64/gdk-pixbuf-2.0/2.10.0/loaders.cache
    to make things work again for the time being.

    (mrv2:6869): Gtk-WARNING **: 09:23:50.244: Theme parsing error: gtk.css:1422:23: 'font-feature-settings' is not a valid property name

    (mrv2:6869): Gtk-WARNING **: 09:23:50.245: Theme parsing error: gtk.css:3308:25: 'font-feature-settings' is not a valid property name

    (mrv2:6869): Gtk-WARNING **: 09:23:50.246: Theme parsing error: gtk.css:3770:23: 'font-feature-settings' is not a valid property name


  Lamentablemente, estos avisos no pueden ser evitados.
  Deberás compilar desde codigo fuente en tu plataforma o usar mrv2 bajo
  XWayland o bajo X11.

- Usand Drag and Drop desde Chrome or Chromium no funciona en Wayland.

  Esto es porque mrv2 corre bajo Wayland mientras que Chromium corre bajo
  XWayland.
  
  Hay un atajo, sin embargo, para trabajar con Wayland: Chromium puede correr
  como cliente de Wayland lanzándolo así::

    chromium --ozone-platform-hint=wayland &

  Con esto, DnD desde el campo de URL de Chromium a mrv2 funciona tanto con
  gnome/Mutter y KDE/Plasma.

  Es posible configurar chromium para que corra como un cliente Wayland cuando sea posible o un cliente X11 de otra forma::

    Corra chromium
    Tipée chrome://flags/#ozone-platform-hint en el campo de URL
    Cambie el seteo "Preferred Ozone platform" a "Auto"
    Cierre y re-lance chromium que correrá como cliente de Wayland.
    
  Note que el soporte de Chrome bajo Wayland tiene muchos errores.
  

Windows
=======

- Luego de instalar con asociaciones de archivos todos los íconos aparecen con
  el logo de mrv2.
  ¿Cómo puedo mostrar la imagen de vista previa?

  * Es un error en Windows, pero se puede evitar.  Deberás
    seleccionar un archivo con la extensión para el que quieres vista previa
    y seleccionar::
    
      Abrir con->Seleccionar otra app

    Luego seleccionar "Fotos" del menú y "Siempre".  Esto restaurá la vista
    previa, pero removerá la asociación a mrv2.  Para asociar mrv2 nuevamente
    al archivo, vaya nuevamente a::

      Abrir con->Seleccionar otra app

    Pero esta vez seleccione "mrv2 Media Player Latest" y "Siempre".  Esto 
    asociará mrv2 de nuevo con el archivo, pero dejará las vistas previas.
    Verá un ícono de mrv2 en la esquina inferior derecha de la vista previa.

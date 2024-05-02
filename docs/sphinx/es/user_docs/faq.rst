####################
Preguntas Frecuentes
####################

All Platforms
=============

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
  
    * Ventana->Preferencias

      - OpenGL
	
	Precisión de Color seteado a Automático (preferido) o Rápido.

      - Línea de Tiempo

	Vista Previa de Miniaturas apagado.

      - Reproducción

	Auto Ocultar Barra de Pixel checked
	o optionalmente ocular la barra de pixel en la Interfaz o
	establecer modo de Presentación.

    * Panel->Settings
      
      - Cache Gigabytes

	Seteado a 4GB por lo menos (por defecto es la mitad de tu memoria)


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

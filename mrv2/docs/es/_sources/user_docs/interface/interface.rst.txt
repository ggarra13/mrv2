.. _interface:

###################
La interfaz de mrv2
###################

.. image:: ../images/interface-02.png
   :align: center

La ventana principal de mrv2 provee de 6 barras diferentes que pueden ser ocultadas o mostradas.

La primer barra es la de los menúes.  Puede alternarse con Shift + F1. Los menus están tambien disponibles con el Botón Derecho del Mouse en la vista principal de mrv2.  La barra de menú tiene también el botón de Edición para alternar el modo de edición y el botón de Ajustar Ventana a la Imagen.

La segunda barra es la de capas o canales, exposición, OCIO y controles de gama.  Puede ser alternada con F1.

La tercera barra es la ventana de la línea de tiempo y sus controles.  Puede alternarla con F3.

La cuarta barra es la de Información de Pixel, que muestra el pixel actual bajo el cursor.  Puede activar y desactivarla con F2.

Finalmente, la últime barra es la de Estatus.  Imprimirá errores y te dejará saber en que modo estás (Por defecto es Scrubbing).


Personalizando la Interfaz
--------------------------

.. image:: ../images/interface-03.png
   :align: center

mrv2 puede ser personalizado para mostrar cualquiera de las barras desde Ventana->Preferencias->Interfaz del Usuario.  Estos seteos son grabados cuando salis de mrv2 y te permitirán arrancar siempre mrv2 con cierta configuración.


La Línea de Tiempo
++++++++++++++++++

.. image:: ../images/timeline-01.png
   :align: center

La Ventana Gráfica de la Línea de Tiempo permite escalar las miniaturas de Edición y las ondas de Audio arrastrando la ventana arriba y abajo.  Para una rápida vista de todas las pistas, puede cliquear en el boton de Edición en la barra de Men.
Cuando se muestran las miniaturas, puedes acercarte o alejarte con la rueda del ratón.

Ocultando/Mostrando Elementos de la GUI 
+++++++++++++++++++++++++++++++++++++++

Algunas teclas útiles por defecto:

============  =============================================
Tecla         Acción
============  =============================================
Shift + F1    Alternar la barra de Menú.
F1            Alternar la barra Superior.
F2            Alternar la barra de Pixel.
F3            Alternar la barra de Línea de Tiempo.
Shift + F7    Alternar las Herramientas de Dibujo y Acción.
F11           Alternar el modo Pantalla Completa.
F12           Alternar el modo Presentación (sin barras).
============  =============================================

Interacción del Ratón en el Visor
---------------------------------

Un ratón de tres botones es recomendado y puede ser usado para inspección de la imagen. Sosteniendo el botón del medio del ratón y moviendo el ratón para panear la imagen en la ventana gráfica. Sostenga la tecla Alt y el botón derecho del ratón y moviendo el ratón de derecha a izquierda para hacer un acercamiento o alejamiento de la imagen.  También puede usar la rueda del mouse que es más confortable.
El factor de zoom actual es mostrado en la barra de pixel a la izquierda.

.. note::
    Para 'resetear' el visor para que la imagen se ajuste a la ventana gráfica, puede seleccionar "Fit" del display de Zoom en la barra de Pixel o usar la tecla 'f'.

.. note::
    Para 'centrar' la vista, sin cambiar el factor de zoom, puede usar la tecla
    'c' hotkey.

.. note::
   Si quiere acercase o alejarse un porcentaje particular (digamos 2x), puede
   elegirlo desde el menu de zoom en la barra de Pixel.

View Menu
---------

The view menu provides controls for modifying the appearance and behaviour of the viewer:

.. topic:: Safe Areas

   The Safe Areas toggle allows you to display the film and video safe areas.
    
.. topic:: Data Window

   Toggling this on will show or hide the OpenEXR's Data Window.
   
.. topic:: Display Window

   Toggling this on will show or hide the OpenEXR's Display Window.	
	   
.. topic:: Mask

    The mask allows drawing a black mask cropping your picture to a certain film aspect.

.. topic:: HUD

    Click this to enter the HUD (heads up display) settings. The HUD allows displaying of a lot of metadata of your media.
      

Timeline
--------

Frame Indicator
+++++++++++++++

Immediately to the left of the timeline is the 'current frame' indicator. Click on this to get a list of options as to how the current time is displayed:
    - *Frames:* absolute frame, i.e. the first frame of the media is always frame 1. 
    - *Seconds:* the playhead position from the start of the media in seconds
    - *Timecode:* the 8 digit timecode. If the media has timecode metadata this will be used

Transport Controls
++++++++++++++++++

These are pretty universal and don't need much explanation. There's a play/pause button, step forwards/backwards buttons and fast forard/fast rewind buttons.

FPS
+++

The frames-per-second (FPS) indicator showing the desired FPS.  The FPS button is a popup that allows you to quickly switch to a new frame rate.

Start and End Frame Indicator
+++++++++++++++++++++++++++++

To the right of the timeline, the Start frame and End Frame indicators are shown.  The S and E buttons can be clicked to set the In and Out points at the current frame.  It is equivalent to pressing the 'I' or 'O' hotkeys.

Player/Viewer Controls
++++++++++++++++++++++

Two buttons to the bottom of the timeline viewport provide the following interactions
    - *Volume/mute control:* click once to get a pop-up volume control. Double click to toggle muting of audio.
    - *Loop mode:* set whether the playhead will loop a source in playback, play it once and stop on the final frame or 'ping-pong' loop.

The Panels
++++++++++

mrv2 supports Panels to organize the information logically.  These panels can be docked to the right of the main viewport or can be made floating windows if dragged from their main drag bar.

Divider
+++++++

The Panels have a divided, just like the Timeline Viewport, and can be dragged to make the panel bigger or smaller (and change the size of the main viewport).




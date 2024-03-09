.. _interface:

###################
La interfaz de mrv2
###################

.. image:: ../images/interface-02.png
   :align: center

La ventana principal de mrv2 provee de 6 barras diferentes que pueden ser ocultadas o mostradas.

La primer barra es la de los menúes.  Puede alternarse con Shift + F1. Los menús están tambien disponibles con el Botón Derecho del Mouse en la vista principal de mrv2.  La barra de menú tiene también el botón de Edición para alternar el modo de edición y el botón de Ajustar Ventana a la Imagen.

La segunda barra es la de capas o canales, exposición, OCIO y controles de gama.  Puede ser alternada con F1.

La tercera barra es la ventana de la línea de tiempo y sus controles.  Puede alternarla con F2.

La cuarta barra es la de Información de Pixel, que muestra el pixel actual bajo el cursor.  Puede activar y desactivarla con F3.

Finalmente, la últime barra es la de Estatus.  Imprimirá errores y te dejará saber en que modo estás (Por defecto es Scrubbing).

Ocultando/Mostrando Elementos de la GUI 
+++++++++++++++++++++++++++++++++++++++

Algunas teclas útiles por defecto:

============  ======================================================
Tecla         Acción
============  ======================================================
Shift + F1    Alternar la barra de Menú.
F1            Alternar la barra Superior.
F2            Alternar la barra de Línea de Tiempo.
F3            Alternar la barra de Pixel.
e             Alternar los dibujos (edición) en la Línea de Tiempo.
Shift + F7    Alternar las Herramientas de Dibujo y Acción.
F11           Alternar el modo Pantalla Completa.
F12           Alternar el modo Presentación (sin barras).
============  ======================================================


Personalizando la Interfaz
--------------------------

.. image:: ../images/interface-03.png
   :align: center

mrv2 puede ser personalizado para mostrar cualquiera de las barras desde **Ventana->Preferencias->Interfaz del Usuario**.  Estos seteos son grabados cuando salis de mrv2 y te permitirán arrancar siempre mrv2 con cierta configuración.

Interacción del Ratón en el Visor
---------------------------------

Un ratón de tres botones es recomendado y puede ser usado para inspección de la imagen. Sosteniendo el botón del medio del ratón y moviendo el ratón para panear la imagen en la ventana gráfica. Sostenga la tecla Alt y el botón derecho del ratón y moviendo el ratón de derecha a izquierda para hacer un acercamiento o alejamiento de la imagen.  También puede usar la rueda del mouse que es más confortable.
El factor de zoom actual es mostrado en la barra de pixel a la izquierda.

.. note::
    Para 'resetear' el visor para que la imagen se ajuste a la ventana gráfica, puede seleccionar "Fit" del display de Zoom en la barra de Pixel o usar la tecla 'f'.

.. note::
    Para 'centrar' la vista, sin cambiar el factor de zoom, puede usar la tecla
    'h' (Home en inglés).

.. note::
   Si quiere acercase o alejarse un porcentaje particular (digamos 2x), puede
   elegirlo desde el menú de zoom en la barra de Pixel.

La Barra Superior
+++++++++++++++++

La Barra Superior de mrv2, contiene los controles de capas o canales de la imágen (usualmente OpenEXR).

La ganancia (controlada con el deslizador) y/o exposición, manejada con las flechas y alternada con el botón que muestra el F-stop (f/8 por defecto).

El Espacio de Entrada de Color está a su lado.  Este es el Control de OpenColorIO (OCIO) de la imágen.

Junto a él, está el control de OpenColorIO (OCIO) del display y de la vista.

El siguiente botón marcado con "L" es el controls de "Looks" de OpenColorIO (OCIO).  Permite agregar un look artístico a la imagen sobre el ya aplicado ACES, por ejemplo.

Finalmente, el último control es el de gama que se controla con el deslizador y se alterna entre dos valores con el botón marcado con "Y".

.. note::

   Los controles de OpenColorIO (OCIO) son derivados de tu archivo de configuración, que es especificado en **Ventana->Preferencias->OCIO**.  Por defecto, el archivo de configuración de OCIO usado es el de cg-config.  mrv2 incluye también el de nuke-default y el de studio-config.
   Si setea la variable de entorno OCIO, esta tendrá precedencia sobre el seteo grabado en el archivo de preferencias de mrv2.

La Línea de Tiempo
++++++++++++++++++

.. image:: ../images/timeline-01.png
   :align: center

La Ventana Gráfica de la Línea de Tiempo permite escalar las miniaturas de Edición y las ondas de Audio arrastrando la ventana arriba y abajo.  Para una rápida vista de todas las pistas, puede cliquear en el boton de Edición en la barra de Men.
Cuando se muestran las miniaturas, puedes acercarte o alejarte con la rueda del ratón.

Indicador de Cuadro
-------------------

Inmediatamente a la izquierda y abajo de la línea de tiempo está el 'cuadro actual'. Junto a él esta un menú de opciones para establecer como el tiempo se muestra:
    - *Cuadros:* cuadros, empezando en 0 para películas o uno normalmente para xsecuencias.
    - *Segundos:* La posición en segundos del medio.
    - *Timecode:* el timecode de 8 digitos. Si el medio tiene metadatos de timecode estos serán usados.

Controles de Transporte
-----------------------

Estos son bastante universales y no necesitan mucha explicación.
Hay un boton de Play para atrás, Pausa y Play para adelante, paso hacia delante o hacia atrás y saltar al comienzo o al final del clip.

FPS
---

El indicador de cuadros por segundo o frames-per-second (FPS) muestra la velocidad de reproducción deseada.  El botónd de FPS es un menú que permite seleccionar rápidamente el cambio a una nueva velocidad.

Start and End Frame Indicator
-----------------------------

A la derecha y abajo de la línea de tiempo, se muestran el cuadro de Comienzo y Final.  Los botones S and E pueden ser cliqueados para establecer el punto de Entrada y Salida en el cuadro actual.  Esto es equivalente a presionar las teclas 'I' o 'O'.

Player/Viewer Controls
----------------------

Dos botones en la parte inferior de los controles de la línea de tiempo proveen las siguientes interacciones:
    - *Volumen/Control mudo:* cliquée en la bocina para alternar el control de mudo. Arrastre el deslizador a su lado para controlar el volumen.
    - *Mode de Bucle:* Estableza si la reproducción se hará una vez y dentendrá en el cuadro final, si se reproducirá en bucle o en 'ping-pong'.

Menú de Vista
+++++++++++++

El menú de Vista provee controles para modificar la apariencia y comportamiento del visor:

.. topic:: Autoencuadre

   El switch de Autoencuadre maneja cómo mrv2 se comporta al cambiar de clips,
   o al redimensionar las ventanas.  Si el Autoencuadre está acivo, la imagen
   siempre se reposicionar para entrar en la vista.  Cuando está apagada,
   el facto de zoom se mantiene al cambiar de un clip a otro.
   
.. topic:: Áreas Seguras

   Alternar Áreas Seguras permite mostrar las áreas seguras de film y video.
    
.. topic:: Ventana de Datos

   Seleccionar esto mostrará o ocultará la Ventana de Datos de OpenEXR.
   
.. topic:: Ventana de Display

   Seleccionar esto mostrará o ocultará la Ventana de Display de OpenEXR.
	   
.. topic:: Ignorar Ventana de Display

   Por defecto, mrv2 recorta los OpenEXRs a la Ventana de Display establecida
   en el archivo.
   Sin embargo, si la Ventana de Datos es más *grande* que la Ventana de
   Display esto puede no ser deseado.
   
.. topic:: Máscara

   La máscara permite dibujar una máscara negra que recorta la imágen hasta darle un aspecto cinematográfico determinado.

.. topic:: HUD

   Seleccione esto para entrar a los seteos del HUD (heads up display). El HUD permite mostrar muchos metadatos de tu clip directamente en la ventana gráfica.
   
      
Menú de Render
++++++++++++++

El menú de Render provee controles para modificar la representación de la imagen en el visor:

.. topic:: Canales

   Puede elegir mostrar los canales de Color, Rojo, Verde, Azul o Alfa independientemente.  Por defecto, puede alternar los canales con las teclas "R", "G", "B" y "A".
    
.. topic:: Voltear

   Con estos dos controles, puede voltear la imagen verticalmente o horizontalmente.
   
.. topic:: Fondo

   Por defecto, mrv2 usa un fondo gris oscuro para mostrar las áreas vacías de la ventana gráfica.  Con esto, puede alternar a mostrar un fondo negro.	
	   
.. topic:: Niveles de Vídeo

   Con este control, puede elegir si los niveles de video del archivo de la película son usados, o si usa el Rango Legal o Completo.

.. topic:: Mezcla Alfa

   Puede seleccionar como se maneja el canal alfa cuando la imagen o vídeo tiene uno.  Puede elegir entre Ninguna, Derecha o Premultiplicada.
      
.. topic:: Filtros de Minificación y Magnificación

   Con estos dos controles, puede seleccionar cómo mrv2 muestra las imágenes cuando está de cerca o de lejos.  Puede elegir en usar un Filtro de Cercanía (Pixelado) o uno Lineal.  El Filtro de Magnificación puede ser alternado con Shift + F.

Menú de Reproducción
++++++++++++++++++++

El menú de reproducción tiene las funciones estándard de reproducción que funcionan igual que los botones de la sección de la línea de tiempo en la interfaz principal.  Además de ello, puedes:

.. topic:: Alternar el Punto de Entrada

	   Con esta opción puedes alternar el punto de entrada del o los clips en la línea de tiempo.

.. topic:: Alternar el Punto de Salida

	   Con esta opción puedes alternar el punto de salida del o los clips en la línea de tiempo.

.. topic:: Ir a/Anotación Previa, Ir A/Anotación Siguiente

	   Una vez que hayas creado una o más anotaciones, podrás usar estas opciones de menu para saltar de un cuadro a otro donde la anotación reside.

.. topic:: Anotación/Borrar, Anotación/Borrar Todas

	   Con estos comandos, una vez creada una o más anotaciones podrás borrar la anotación en el cuadro actual o todas las anotaciones de la línea de tiempo.

Menú de Línea de Tiempo
+++++++++++++++++++++++

El menú de línea de tiempo provee controles para modificar la ventana gráfica de la línea de tiempo en la parte inferior de la ventana de vista:

.. topic:: Editable

   Cuando está activada, podrás mover varios clips creados con el Panel de Lista de Reproducción, la herramienta Editar/Rebanar o cuando se lea un archivo .otio.  La parte superior de la línea de tiempo (aquella con números), te permitirá ir de un cuadro a otro.  Cuando no está activa, puedes cliquear en cualquiera de las imágenes y eso también te llevará a otro cuadro. 
    
.. topic:: Edit Clips Asociados.

   Cuando este control está activado, clips de vídeo y audio pueden ser
   movidos juntos si comienzan y terminan *EXACTAMENTE* en el mismo lugar.
   Nótese que es en general díficil lograr que los tracks de audio y video
   empaten exáctamente.
   
.. topic:: Miniaturas

   Este seteo te permite desactivar los dibujos en minitura de la ventana
   gráfica de la línea de tiempo, así como seleccionar tamaños más grandes si
   tienes un monitor con más alta resolución.
	   
.. topic:: Transitiones

   Con esto prendido, podés mostrar las transiciones de audio y vídeo en
   archivos .otio.
   (Actualmente no implementado en v1.0.9).

.. topic:: Marcadores

   Con este seteo prendido, podés mostrar marcadores .otio en la ventana
   gráfica de la línea de tiempo.
   Los marcardores son usados en archivos .otio para marcar areas interesantes
   en la línea de tiempo.
   
Menú de Imagen
++++++++++++++

Este menú aparece solo cuando up clip con version es detectado en el disco.  Por defecto, esto es un directorio, archivoe o ambos nombrados con "_v" y un número, como::

  Fluid_v0001.0001.exr
  Bunny_v1/Bunny.0001.exr

Nótese que esto se puede cambiar con expression regular (regex) en Ventana->Preferencias->Cargando.

.. topic:: Version/Primera, Version/Última

	   Chequeará en el disco por la primera o última version en el disco.  Por defecto, aceptará un máximo de 10 versiones antes de rendirse.  Puede ver como empata el clip en panel de Bitácora o en la consola si arrancó mrv2 en la línea de comandos.

.. topic:: Version/Previous, Version/Next

	   Buscará la siguiente o previa version en el disco.  Por defecto, aceptará un máximo de 10 versiones antes de rendirse.  Puede ver como empata el clip en panel de Bitácora o en la consola si arrancó mrv2 en la línea de comandos.
  
Menú de Editar
++++++++++++++

El menú de editar provee funcionalidad rápida para editar la línea de tiempo y los clips.  No intenta ser un Editor No Lineal completo, pero sí una forma de testear tus cambios y ajustar tus animaciones.

.. topic:: Cuadro/Cortar, Cuadro/Copiar, Cuadro/Pegar, Cuadro/Insertar

	   Estos controles te permiten cortar, copiar, pegar e insertar un solo cuadro de animación.  Es útil para animadores para bloquear su timing, sin tener que necesariamente abrir el paquete de animación y ajustar múltiples curvas.
    
.. topic:: Brecha de Audio/Insertar, Brecha de Audio/Remover

	   Esta opción de menú permite agregar una brecha de audio de una porción de vídeo que no tiene audio.  Posiciónese en la línea de tiempo sobre el cuadro del clip al que le quiere agregar la brecha y seleccione Insertar.  Para removerla, haga lo mismo pero use Remover.
   
.. topic:: Rebanar

	   Este comando rebanará (cortará) el o los clip(s) en el cuadro actual de la línea de tiempo, creando dos clips.
	   
.. topic:: Remover

	   Esta opcion removerá el o los clips en la posición actual de la línea de tiempo.

.. topic:: Deshacer/Rehacer

	   Estos comando deshacen o rehacen la última edición.  No deben de confundirse con el Deshacer y Rehacer de las anotaciones.

Los Paneles
+++++++++++

mrv2 soporta Paneles para organizar la información lógicamente.  Estos paneles pueden ser empotrados a la derecha de la ventana gráfica o ser ventanas flotatantes si se las arrstra de su barra superior o se presiona en el pequeño botón amarillo.

Divisor
+++++++

Los Paneles tienen un divisor, tal como la Ventana Gráfica de la Línea de Tiempo, que puede ser arrastrado para hacer el panel mas grande o pequeño (y así también cambiar el tamaño de la ventana gráfica principal).




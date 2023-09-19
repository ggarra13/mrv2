#####################
Reproducción de Vídeo
#####################

Cuadro Actual
-------------

En mrv2, cada clip y lista de reproducción tiene su propio cuadro actual lo que significa que si cambia de uno a otro, cosas como la velocidad de reproducción, el modo de bucle, etc. son recordados *por lista de reproducción* o clip. 

Modos de Bucle
--------------

Use botón de modo de bucle para cambiar entre 'reproducir una vez', 'reproducir en bucle' y 'ping-pong' cuando se reproduce el medio.

Velocidad de FPS
----------------

La velocidad de FPS (Frames per Second o Cuadros por Segundo) puede ser ajustada desde el botón de 'FPS' en la barra de playback.  También puede escribir una velocidad al azar en el widget de FPS.

Teclas Específicas de Reproducción
----------------------------------

=======================  =====================================
Teclas                   Acción
=======================  =====================================
Barra Espaciadora        Comienza o detiene la reproducción.
I                        Alterna el punto de entrada de bucle.
O                        Alterna el punto de salida de bucle.
Flecha Arriba            Reproducir hacia atrás.
Entrar                   Detener reproducción.
Flecha Abajo             Reproducir hacia delante.
Flecha Derecha           Caminar hacia delante un cuadro.
Flecha Izquierda         Caminar hacia atrás un cuadro.
Shift+Flecha Izquierda   Ir a la anotación previa.
Shift+Flecha Derecha     Ir a la próxima anotación.
=======================  ==================================== 

Comportamiento del Cache
------------------------

mrv2 tratará siempre de leer y decodificar datos de vídeo antes de que sea necesitado para el display. Los datos de la imagen son guardados en el cache listo para dibujar en la pantalla. mrv2 necesita ser eficiente en cómo hace esto y es útil si el usuario entiende su comportamiento.

El estatus del cache está indicado en la línear de tiempo por una delgada línea de color horizontal - esto debe ser obvio ya que puede verla crecer mientra mrv2 carga cuadros del disco en el fondo. Por ello, si quiere ver un clip que es lento de leer del disco, como unas imágenes de EXR de alta resolución, el comportamiento es esperar que mrv2 cache los cuadros antes de arrancar la reproducción/bucle. El tamaño del cache (seteado via el Panel de Seteos) limitará el máximo número de cuadros que pueden ser cargados. 

En la mayoría de los casos, mrv2 debería ser capaz de reproducir los cuadros en el cache a la velocidad de FPS del medio. A pesar que el Visor ha sido optimizado para obtener lo mejor de su tarjeta gráfica, la reproducción lenta puede resultar si esta tratando de ver imágenes de muy alta resolución y el hardware de video de su computadora no puede empatar el ratio de transferencia.

Para medios que pueden ser decodificados más rápido que el ratio de reproducción, como muchos codecs de video o EXRs comprimidos con DWA/DWB, debería poder ignorar la actividad de caching ya que éste será capaz de correr los datos del disco para reproducción en demanda.


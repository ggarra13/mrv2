============
Introducción
============


¿Qué es mrv2?
*************

mrv2 es un flipbook profesional y herramienta de revisión de código abierto para la industria de Efectos Visuales, Animación e Imágenes de Computación Gráfica.  mrv2 esta focalizado en proveer una interfaz fácil de usar e intuitiva con el motor de reproducción de mayor performance disponible y una API de Python para integración en los Pipelines de los estudios y para customizarlo con total facilidad.

mrv2 puede manejar colecciones de medios rápidamente, carga una multitud de formatos de imagen y video especializados y muestra la imágenes con manejo de color. Los usuarios pueden importar, organizar y agrupar medios en listas de reproducción de OTIO, reproduciendo items y agregando notas de revisiones y anotaciones dibujadas, permitiendo visualizar el contenido en forma interactiva y colaborativa.  Esto permite el flujo que es esencial para los equipos de efectos visuales, animación y otras actividades de post-production que necesitan ver, en demanda, el arte que ellos y sus colegas están creando.  Por ejemplo, uno puede cambiar de fuente de medios instantáneamente, inspeccionar pixels de cerca, hacer comparaciones cuadro a cuadro a través de multiple fuentes de medios, anotar los medios con dibujos y texto o agregar notas para compartir con otros.


Versión Actual: v1.1.0 - Descripción General
********************************************

Esta versión de la aplicación es una solución de revisión sólida y robusta. mrv2 ha sido desplegado en múltiples estudios y es usado por múltiples individuos diariamente desde Augosto del 2022.

Aquí está un resumen de ellas:

**Cargar Medios**

  - Muestre virtualmente cualquier formato de imágen en uso hoy (EXR, TIF, TGA, JPG, PSD, MOV, MP4, WEBM, etc).
  - Use Drag and drop de los medios del sistema de archivos del escritorio directo a la interfaz de mrv2.
  - Use la API de Python para construir listas de reproducción con sus propios scripts, controle el reproductor, grabe películas, compare clips y control su integración de su pipeline.
  - Reproducción de audio para fuentes con una pista de audio es provista con fregado y playback en reverso.
  - Soporte nativo de .otio permite reproducir líneas de tiempo de OpenTimelineIO con fundidos.
  - Soporte de USD (Universal Scene Description) de Pixar en OpenGL.
  - Comunicación en red de NDI.

**Listas de reproducción**

  - Cree cualquier número de listas de reproducción, grabándolas en un archivo .otio y editandolo con las herramientas de edición provistas.

**Annotaciones y Notas**

  - Agregue notas y annotaciones a los medios en cuadros individuales frames o en todos los cuadros.
  - Annotaciones en la pantalla pueden ser creadas con herramientas simples de dibujo.  Las características de las anotaciones actualmente incluyen:
      
    1. Color ajustable, opacidad, suavizado y tamaño de los pinceles. 
    2. Herramientas para rectángulos, círculos, líneas y flechas. 
    3. Goma de borrar para más flexibilidad de dibujo..
    4. Texto editable con tipografías ajustables, posición, tamaño, color y
       opacidad.  El texto soporta UTF-8 para texto internacional
       (Japonés, etc).
    5. Grabe las anotaciones a disco como una película o con cualquiera de los
       formatos soportados.
       
  - Navegue su colección de archivos a con notas saltando directamente de un
    cuadro a otro.
  - Exporte las notas y anotaciones a un documento PDF para compartir con
    productores.

.. image:: images/interface-01.png

**El Visor**

  - Display preciso de color (OCIO v2 colour management).
  - Interacción con teclas.
  - Ajuste la exposición y la velocidad de reproducción.
  - Herramientas de correción de color para controlar ganancia, gama, tinte,
    saturación, etc.
  - Zoom/pan, y display individuales de canales RGBA.
  - Modos de Comparación: A/B, Wipe, Overlay, Difference, Horizontal, Vertical
    y Mosaico.
  - Sobreposición de máscaras predefinidas y guías de áreas seguras.
  - Segundo visor desplegable para uso con multiples monitores.

**Sesiones**

  - Sincronismo de sesiones está provisto para sincronizar uno o más visores en una sesión de revision en una LAN (Red de Área Local).  Puede tener un servidor y múltiples clientes y pueden controlar todos los aspectos de mrv2 (seleccionable por el usuario).
  - Archivos de Sesiones (.mrv2s) pueden ser grabados a disco para grabar el estado de la interfaz y los clips cargados.
    
**Teclas de manejo**

  - Teclas de manejo definidas por el usuario grabadas en un archivo separado
    (mrv2.keys.prefs) permite cambiar todos los seteos de los menúes y algunos
    controles del visor.
    
**Características de la API para desarrolladores de pipelines**

*API de Python*

  - Un intérprete empotrable de Python está disponible para ejecutar scripts dentro de mrv2, agregar o crear nuevas entradas en los menúes.
  - Controle todos los paneles y la línea de tiempo.


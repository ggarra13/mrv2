============
Introduction
============


What is mrv2 ?
**************

mrv2 es un flipbook profesional y herramienta de revisión de código abierto para la industria de Efectos Visuales, Animación e Imágenes de Computación Gráfica.  mrv2 esta focalizado en proveer una interfaz fácil de usar e intuitiva con el motor de reproducción de mayor performance disponible y una API de Python para integración en los Pipelines de los estudios y para customizarlo con total facilidad.

mrv2 puede manejar colecciones de medios rápidamente, carga una multitud de formatos de imagen y video especializados y muestra la imágenes con manejo de color. Los usuarios pueden importar, organizar y agrupar medios en listas de reproducción de OTIO, reproduciendo items y agregando notas de revisiones y anotaciones dibujadas, permitiendo visualizar el contenido en forma interactiva y colaborativa.  Esto permite el flujo que es esencial para los equipos de efectos visuales, animación y otras actividades de post-production que necesitan ver, en demanda, el arte que ellos y sus colegas están creando.  Por ejemplo, uno puede cambiar de fuente de medios instantáneamente, inspeccionar pixels de cerca, hacer comparaciones cuadro a cuadro a través de multiple fuentes de medios, anotar los medios con dibujos y texto o agregar notas para compartir con otros.


Current Version: v0.8.0 - Overview
**********************************

Esta versión de la aplicación es una solución de revisión sólida y robusta. mrv2 ha sido desplegado en múltiples estudios y es usado por múltiples individuos diariamente desde Augosto del 2022.

La fase de desarrollo está todavía en pleno funcionamiento mientras el trabajo continua en algunas características mayores. 

Aquí está un resumen de ellas:

**Cargar Medios**

  - Muestre virtualmente cualquier formato de imágen en uso hoy (EXR, TIF, TGA, JPG, PSD, MOV, MP4, WEBM, etc).
  - Use Drag and drop de los medios del sistema de archivos del escritorio directo a la interfaz de mrv2.
  - Use la API de Python para construir listas de reproducción con sus propios scripts, controle el reproductor, grabe películas, compare clips y control su integración de su pipeline.
  - Reproducción de audio para fuentes con una pista de audio es provista con fregado y playback en reverso.
  - Soporte nativo de .otio permite reproducir líneas de tiempo de OpenTimelineIO con fundidos.
  - Soporte de USD (Universal Scene Description) de Pixar en OpenGL.

**Listas de reproducción**

  - Cree cualquier número de listas de reproducción, grabándolas en un archivo .otio y editandolo con las herramientas de edición provistas.

**Annotations and Notes**

  - Add notes and annotations to media on individual frames or range of frames.
  - On screen annotations can be created with easy to use, highly responsive sketching tools. Annotations features currently include:
      
    1. Adjustable colour, opacity and size of brush strokes. 
    2. Shapes tool for boxes, circles, lines and arrows etc. 
    3. Eraser pen for even more sketching flexibility.
    4. Editable text captions with adjustable fonts, position, scale, color
       and opacity.
    5. Save annotations to disk as a movie file or any of the supported image
       saving formats.
       
  - Navigate your media collection through the notes interface by jumping directly to bookmarked frames.
  - Annotations and Notes can be saved as a PDF document for easily sharing with the production staff.

.. image:: images/interface-01.png

**The Viewer**

  - Color accurate display (OCIO v2 colour management).
  - Hotkey driven interaction.
  - Adjust exposure and playback rate.
  - Color correction tools to control gain, gamma, tint, saturation, etc.
  - Zoom/pan image tools, RGBA channel display.
  - A/B, Wipe, Overlay, Difference, Horizontal, Vertical and
    Tile 'Compare Modes'.
  - Predefined masking overlay and safe areas guide-lines.
  - 'Pop-out' 2nd viewer for dual display set-ups.

**Sessions**

  - Session syncing is provided to synchronize one or more viewers in a review session across a LAN.  You can have a server and multiple clients and they can all control all aspects of mrv2 (user selectable).
  - Session files can be saved to disk to save the state of the UI and loaded media.
    
**API features for pipeline developers**

*Python API*

  - An embedded Python interpreter is available to execute scripts within mrv2, add or create new menu entries.
  - Create and build media playlists through straightforward API methods.
  - Control playback (start, stop, step frame, seek, etc).


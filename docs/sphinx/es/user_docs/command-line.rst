#############################################
Switches de la Línea de Comandos y Parámetros
#############################################

mrv2

    Reproduzca líneas de tiempo, películas, y sequencias de imágenes.

Uso:

    mrv2 [inputs] ... [opcion]...

Argumentos:

    inputs
        Líneas de tiempo, películas, sequencias de imágenes o carpetas.

Opciones:

    -audio, -a (value)
        Nombre de archivo de audio.

    -compare, -b (value)
        A/B comparison "B" file name.

    -compareMode, -c (value)
        A/B comparison mode.
        Valor por defecto: A
        Valores posibles: A, B, Wipe, Overlay, Difference, Horizontal, Vertical, Tile

    -wipeCenter, -wc (value)
        A/B centro de comparación del limpiaparabrisas.
        Valor por defecto: 0.5,0.5

    -wipeRotation, -wr (value)
        A/B rotación del limpiaparabrisas.
        Valor por defecto: 0

    -otio, -o, -edl
        Crea un EDL de OpenTimelineIO EDL de la lista de clips provistos.

    -editMode, -e
        Mode de Edición de OpenTimelineIO.

    --single, -single, -s
        Cargar las imágenes como imágenes quietas, no secuencias.

    -speed (value)
        Velocida de reproducción.

    -playback, -p (value)
        Modo de Playback.
        Valor por defecto: (Preferencias)
        Valores Posibles: Stop, Forward, Reverse

    -loop (value)
        Modo de Loop Reproducción.
        Valor por defecto: Loop
        Valores Posibles: Loop, Once, Ping-Pong

    -seek (value)
        Salte al tiempo dado, en formato valor/fps.  Ejemplo: 50/30.

    -inOutRange, -inout (value)
        Setear los puntos de entrada/salida en formato comienzo/final/fps,
	como 23/120/24.

    -ocioInput, -ics, -oi (value)
        Espacio de Color de Entrada de OpenColorIO.

    -ocioDisplay, -od (value)
        Nombre de Display de OpenColorIO.

    -ocioView, -ov (value)
        Nombre de Vista de OpenColorIO.

    -ocioLook, -ol (value)
        OpenColorIO look name.

    -lut (value)
        Nombre de Archivo de LUT.

    -lutOrder (value)
        Órden de Operación de LUT.
        Valor por defecto: PostColorConfig
        Valores Posibles: PostColorConfig, PreColorConfig

    -pythonScript, -ps (value)
        Script de Python a correr y salir.

    -pythonArgs, -pa (value)
        Argumentos de python a pasar al script de python en una cadena de
	comillas, como "arg1 'arg2 asd' arg3".  Se guarda en cmd.argv.

    -resetSettings
        Resetear seteos a su defecto.

    -resetHotkeys
        Resetear teclas de manejo a su defecto.

    -usd, -usdOverrides (value)
        Overrides de US.
        Valor por defecto: 0

    -usdRenderWidth (value)
        Valor de Tamaño de Render de USD.
        Valor por defecto: 1920

    -usdComplexity (value)
        Seteo de complejidad del render de USD.
        Valor por defecto: 1

    -usdDrawMode (value)
        Modo de dibbujo del render de USD.
        Valor por defecto: ShadedSmooth
        Valores posibles: Points, Wireframe, WireframeOnSurface, ShadedFlat, ShadedSmooth, GeomOnly, GeomFlat, GeomSmooth

    -usdEnableLighting (value)
        Valor de activar luces en USD render.
        Valor por defecto: 1

    -usdEnableSceneLights (value)
        Valor de activar luces en escena en USD render.
        Valor por defecto: 0

    -usdEnableSceneMaterials (value)
        Valor de activar materiales en escena en USD render.
        Valor por defecto: 1

    -usdSRGB (value)
        Seteo de sRGB en rendereo USD.
        Valor por defecto: 1

    -usdStageCache (value)
        Tamaño de cache de escenario de USD.
        Valor por defecto: 10

    -usdDiskCache (value)
        Tamaño de cache de disco de USD en gigabytes.  Un tamaño de cero
	desactiva el cache.
        Valor por defecto: 0

    -server
        Comenzar un servidor.  Use -port para especificar un número de puerto.

    -client (valor)
        Conéctese a un servidor en <valor>.  Use -port para especificar
	un número de puerto.

    -port (value)
        Número de puerto para el servidor para escuchar or para conectar como
	cliente.
        Valor por defecto: 55150

    -version, --version, -v, --v
        Returne la version and sale.

    -log
        Imprimir la bitácora a la consola.

    -help, -h, --help, --h
        Mostrar este mensaje.

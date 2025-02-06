####################################
Command-line Switches and Parameters
####################################

mrv2

    Play timelines, movies, and image sequences.

Usage:

    mrv2 [inputs] ... [option]...

Arguments:

    inputs
        Timelines, movies, image sequences, or folders.

Options:

    -audio, -a (value)
        Audio file name.

    -compare, -b (value)
        A/B comparison "B" file name.

    -compareMode, -c (value)
        A/B comparison mode.
        Default value: A
        Possible values: A, B, Wipe, Overlay, Difference, Horizontal, Vertical, Tile

    -wipeCenter, -wc (value)
        A/B comparison wipe center.
        Default value: 0.5,0.5

    -wipeRotation, -wr (value)
        A/B comparison wipe rotation.
        Default value: 0

    -otio, -o, -edl
        Create OpenTimelineIO EDL from the list of clips provided.

    -editMode, -e
        OpenTimelineIO Edit mode.

    --single, -single, -s
        Load the images as still images not sequences.

    -speed (value)
        Playback speed.

    -playback, -p (value)
        Playback mode.
        Default value: Stop
        Possible values: Stop, Forward, Reverse

    -loop (value)
        Playback loop mode.
        Default value: Loop
        Possible values: Loop, Once, Ping-Pong

    -seek (value)
        Seek to the given time, in value/fps format.  Example: 50/30.

    -inOutRange, -inout (value)
        Set the in/out points range in start/end/fps format, like 23/120/24.

    -ocioInput, -ics, -oi (value)
        OpenColorIO input color space.

    -ocioDisplay, -od (value)
        OpenColorIO display name.

    -ocioView, -ov (value)
        OpenColorIO view name.

    -ocioLook, -ol (value)
        OpenColorIO look name.

    -lut (value)
        LUT file name.

    -lutOrder (value)
        LUT operation order.
        Default value: PostColorConfig
        Possible values: PostColorConfig, PreColorConfig

    -pythonScript, -ps (value)
        Python Script to run and exit.

    -pythonArgs, -pa (value)
        Python Arguments to pass to the Python script as a single quoted string like "arg1 'arg2 asd' arg3", stored in cmd.argv.

    -resetSettings
        Reset settings to defaults.

    -resetHotkeys
        Reset hotkeys to defaults.

    -usd, -usdOverrides (value)
        USD overrides.
        Default value: 0

    -usdRenderWidth (value)
        USD render width.
        Default value: 1920

    -usdComplexity (value)
        USD render complexity setting.
        Default value: 1

    -usdDrawMode (value)
        USD render draw mode.
        Default value: ShadedSmooth
        Possible values: Points, Wireframe, WireframeOnSurface, ShadedFlat, ShadedSmooth, GeomOnly, GeomFlat, GeomSmooth

    -usdEnableLighting (value)
        USD render enable lighting setting.
        Default value: 1

    -usdEnableSceneLights (value)
        USD render enable scene lights setting.
        Default value: 0

    -usdEnableSceneMaterials (value)
        USD render enable scene materials setting.
        Default value: 1

    -usdSRGB (value)
        USD render SRGB setting.
        Default value: 1

    -usdStageCache (value)
        USD stage cache size.
        Default value: 10

    -usdDiskCache (value)
        USD disk cache size in gigabytes. A size of zero disables the cache.
        Default value: 0

    -server
        Start a server.  Use -port to specify a port number.

    -client (value)
        Connect to a server at <value>.  Use -port to specify a port number.

    -port (value)
        Port number for the server to listen to or for the client to connect to.
        Default value: 55150

    -version, --version, -v, --v
        Return the version and exit.

    -log
        Print the log to the console.

    -help, -h, --help, --h
        Show this message.


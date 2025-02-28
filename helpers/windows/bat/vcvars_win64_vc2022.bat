
REM Set the Visual Studio Environment.  Change this to the location of your
REM Visual Studio. This file is for compiling in Windows 64 bits.
REM
REM This file points to MSVC 2022 Community Edition.
REM
REM You can also call this command with an additional parameter to the SDK
REM to use.
REM
REM By default it uses the latest Win 10 SDK (10.0.20348.0).
REM


@call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 10.0.20348.0

@call C:\msys64\msys2_shell.cmd -use-full-path

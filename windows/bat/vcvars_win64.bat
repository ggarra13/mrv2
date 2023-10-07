
REM Set the Visual Studio Environment.  Change this to the location of your
REM Visual Studio. This file is for compiling in Windows 64 bits.
REM
REM You can also call this command with an additional parameter to the SDK
REM to use, like 8.1. or nothing to pick the default SDK.
REM
REM 8.1 allows you to create an executable which is compatible with Windows
REM 8.1, 10, and 11.
REM

@call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 

C:\msys64\msys2_shell.cmd -use-full-path

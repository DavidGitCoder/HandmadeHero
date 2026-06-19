@echo off

REM create a directoy outside of the project
mkdir ..\..\build
REM moves temporarily to that /build
pushd ..\..\build

REM compiles the cpp file
cl -Zi ..\HandmadeHero\code\win32_handmade.cpp user32.lib

REM returns to current directory
popd

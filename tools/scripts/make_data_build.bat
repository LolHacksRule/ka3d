@echo off
set APPNAME=data-.zip
set VERTAG=version

lua path="../../core/data/shaders/readme.txt" vertag="%VERTAG%" prj="shaders" ..\scripts\vername.lua
echo Build number incremented?
pause


rem -- create ziptmp
md ziptmp

rem -- shaders
xcopy ..\..\core\data ziptmp  /S /I


rem -- zip and move to builds output directory
cd ziptmp
pkzip -add -dir %APPNAME%
move %APPNAME% "%HOME%\_bak\builds"
cd ..

rem -- delete ziptmp
delfiles -d ziptmp

rem -- append version numbers to zip base name
lua path="../../core/data/shaders/readme.txt" vertag="%VERTAG%" prj="shaders" file="%HOME%\_bak\builds\%APPNAME%" ..\scripts\vername.lua

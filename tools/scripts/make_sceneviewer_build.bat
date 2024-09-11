@echo off
set APPNAME=hgrviewer-.zip

lua prj="sceneviewer" ..\scripts\vername.lua
echo Build number incremented?
pause

rem -- create ziptmp
md ziptmp

rem -- sceneviewer
copy ..\docs\hgrviewer.txt ziptmp
copy ..\build\msvc7\sceneviewer\release\sceneviewer.exe ziptmp\hgrviewer.exe


rem -- zip and move to builds output directory
cd ziptmp
pkzip -add -dir %APPNAME%
move %APPNAME% "%HOME%\_bak\builds"
cd ..

rem -- delete ziptmp
delfiles -d ziptmp

rem -- append version numbers to zip base name
lua prj=sceneviewer file="%HOME%\_bak\builds\%APPNAME%" ..\scripts\vername.lua

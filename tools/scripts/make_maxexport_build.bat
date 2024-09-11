@echo off
set APPNAME=3dsmax-hgrexport-.zip

lua prj="maxexport" ..\scripts\vername.lua
echo Build number incremented?
pause

rem -- create ziptmp
md ziptmp

rem -- maxexport
copy ..\docs\hgrexport.txt ziptmp
copy ..\build\msvc7\maxexport5\release\*.dle ziptmp
copy ..\build\msvc7\maxexport6\release\*.dle ziptmp
copy ..\build\msvc7\maxexport7\release\*.dle ziptmp
copy \core\source\img\Devil-SDK-1.6.7\lib\DevIL.dll ziptmp


rem -- zip and move to builds output directory
cd ziptmp
pkzip -add -dir %APPNAME%
move %APPNAME% "%HOME%\_bak\builds"
cd ..

rem -- delete ziptmp
delfiles -d ziptmp

rem -- append version numbers to zip base name
lua prj=maxexport file="%HOME%\_bak\builds\%APPNAME%" ..\scripts\vername.lua

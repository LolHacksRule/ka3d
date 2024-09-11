--
-- Opens version.h from prj directory and 
-- gets value of #define <app>_VERSION
--
-- Platform:
-- Platform independent (tested on WinXP)

-- Assumes:
-- version.h exists and has <app>_VERSION_NUMBER defined
--
-- Parameters:
--
-- prj
--   Name of the project directory.
--
-- file (optional)
--   If 'file' is not set then version number is printed.
--   If 'file' is set then file base name is appended
--   with version identifier found the version.h.
--
-- path (optional)
--   Path to 'version.h' file (including file name).
--   If not defined then ../include/<prj> is used.
--
-- vertag (optional)
--   Version number tag. Default is <prj>_VERSION_NUMBER
--
-- Copyright (C) 2004 Jani Kajala. All rights reserved.
--

if ( path == nil ) then
	path = "../include/" .. prj .. "/version.h"
end

removequotes = nil
if ( vertag == nil ) then
	vertag = strupper(prj) .. "_VERSION_NUMBER"
	removequotes = 1
end

fh = openfile( path, "rt" )
if ( fh == nil ) then
	write( "version.h not found\n" )
end

w = read( fh, "*w" )
while ( w ~= nil ) do
	--print( "w = " .. w )
	--print( "Finding " .. vertag )
	index = strfind( w, vertag )
	if ( index ~= nil )
	then
		w = read( fh, "*w" )
		if ( w ~= nil ) then
			--print( "w2 = " .. w )
			ver = w
			if ( removequotes ) then
				ver = strsub(w,2,strlen(w)-1) -- found "1.1.1", cut quotes
			end
			if ( ver ~= nil ) then
				if ( file ~= nil and strlen(file) > 4 and strbyte(file,strlen(file)-3) == strbyte('.') ) 
				then
					base = strsub( file, 1, strlen(file)-4 )
					ext = strsub( file, strlen(file)-3, strlen(file) )
					rename( file, base .. ver .. ext )
					break
				else
					write( "Version id is " .. ver .. "\n" )
					break
				end
			end
		end
	end

	w = read( fh, "*w" )
end

closefile( fh )

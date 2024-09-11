--
-- Backups sources and project files, excludes intermediate files.
--
-- Platform:
-- Windows (tested on WinXP)
--
-- Input:
-- file    Path to backup file without date string + zip suffix.
--
-- Assumes:
-- pkzip exists on path
--
-- Author: Jani Kajala (jani.kajala@helsinki.fi) 5/14/2003
--
execute( "pkzip -add -dir -excl=*.o -excl=*.dll -excl=*.lib -excl=*.exe -excl=*.lst -excl=*.elf -excl=*.map -excl=*.obj -excl=*.opt -excl=*.ncb -excl=*.ilk -excl=*.pdb -excl=*.idb -excl=*.pch -excl=*.plg -excl=*.bsc -excl=*.sbr -excl=*.clw -excl=*.class -excl=*.ctf -excl=*.log -excl=*.exp -excl=*.dvi tmp" )
execute( "move tmp.zip \"" .. file .. date("%Y%m%d") .. ".zip\"" )

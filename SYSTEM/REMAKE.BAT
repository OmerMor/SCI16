@echo off

if "%1" == "+b" goto backup
if "%2" == "+b" goto backup
if "%1" == "-b" goto nobackup
if "%2" == "-b" goto nobackup

rem default is
goto nobackup

:backup
call backsys

:nobackup

del *.hep
del *.scr
del *.tex

del classdef
copy selector.new selector

if "%1" == "-l" goto Log
if "%1" == "+l" goto Log
if "%1" == "l" goto Log

rem need files vocabase.vc deriv.txt grammar.txt nlsyms.txt
rem compile vocab.vc using vcomp to test vocabase.vc
rem
call x:sc -O @remake.lst
goto Exit

:Log
echo.                         > remake.log
echo Remaking system          >> remake.log
echo.                         >> remake.log
call x:sc -O @remake.lst 		>> remake.log

:Exit

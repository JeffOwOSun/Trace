@echo off
setlocal enableDelayedExpansion

set INPUTDIR=input
set OUTPUTDIR=output
FOR %%i in (%INPUTDIR%\*.ray) do (
    set OUTPUTFILE=%OUTPUTDIR%\%%~ni.bmp
    ray -w 512 "%%i" "!OUTPUTFILE!"
)

@echo off
path="C:\Program Files (x86)\SEGGER\JLink_V496\"

:OPTIONS
echo Select the binary file to load into QSPI memory:
echo 1 Debug
echo 2 Release
echo 3 Exit
SET /P REPLY=">"
if "%REPLY%"== "1" (goto DEBUG)
if "%REPLY%"== "2" (goto RELEASE)
if "%REPLY%"== "3" (goto END)
echo Error: Invalid option!
goto OPTIONS

:DEBUG
if not exist %PATH%\jlink.exe goto ERROR1
if not exist debug\internet_radio_demo.bin goto ERROR2

echo Remove power (5V) to the board before continuing. 
echo Set SW6 as instructed below:
echo SW6-1 OFF, SW6-2 ON, SW6-3 OFF, SW6-4 ON, SW6-5 ON, SW6-6 ON
echo Reconnect power (5V) to the board before continuing. 
pause
jlink.exe -speed 12000 -if JTAG -device R7S721001_DualSPI -CommanderScript load_flash_debug.jlink
goto END

:RELEASE
if not exist %PATH%\jlink.exe goto ERROR1
if not exist release\internet_radio_demo.bin goto ERROR2

echo Remove power (5V) to the board before continuing. 
echo Set SW6 as instructed below:
echo SW6-1 OFF, SW6-2 ON, SW6-3 OFF, SW6-4 ON, SW6-5 ON, SW6-6 ON
echo Reconnect power (5V) to the board before continuing. 
pause
jlink.exe -speed 12000 -if JTAG -device R7S721001_DualSPI -CommanderScript load_flash_release.jlink
goto END

:ERROR1
echo Error: Please check the installation path of JLink!
goto END

:ERROR2
echo Error: The specified binary file does not exist!
goto END

:END
pause

@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION
setlocal
REM go to the folder where this bat script is located
cd /d %~dp0

set ARDUINO_CLI=arduino-cli
set /A EXPECTED_CLI_MAJOR=0
set /A EXPECTED_CLI_MINOR=21

where /q %ARDUINO_CLI%
IF ERRORLEVEL 1 (
    ECHO Cannot find 'arduino-cli' in your PATH. Install the Arduino CLI before you continue.
    ECHO Installation instructions: https://arduino.github.io/arduino-cli/latest/
    EXIT /B
)

REM parse arduino-cli version
FOR /F "tokens=1-3 delims==." %%I IN ('arduino-cli version') DO (
    FOR /F "tokens=1-3 delims== " %%X IN ('echo %%I') DO (
        set /A CLI_MAJOR=%%Z
    )
    SET /A CLI_MINOR=%%J
    FOR /F "tokens=1-3 delims== " %%X IN ('echo %%K') DO (
        set /A CLI_REV=%%X
    )
)

if !CLI_MINOR! LSS !EXPECTED_CLI_MINOR! (
    GOTO UPGRADECLI
)

if !CLI_MAJOR! NEQ !EXPECTED_CLI_MAJOR! (
    echo You're using an untested version of Arduino CLI, this might cause issues (found: %CLI_MAJOR%.%CLI_MINOR%.%CLI_REV%, expected: %EXPECTED_CLI_MAJOR%.%EXPECTED_CLI_MINOR%.x )
) else (
    if !CLI_MINOR! NEQ !EXPECTED_CLI_MINOR! (
        echo You're using an untested version of Arduino CLI, this might cause issues (found: %CLI_MAJOR%.%CLI_MINOR%.%CLI_REV%, expected: %EXPECTED_CLI_MAJOR%.%EXPECTED_CLI_MINOR%.x )
    )
)

set OUTPUT=""
for /f "delims=" %%i in ('arduino-cli config dump ^| findstr /r "user: "') do (
    set OUTPUT=%%i
)
IF "%OUTPUT%" == "" GOTO LIBRARYDIRNOTFOUND

set arduinodirectory=%OUTPUT:~8%%
set library=%OUTPUT:~8%\libraries
echo %arduinodirectory%

echo Finding Arduino MBED core v9.9.9...

(arduino-cli core list  2> nul) | findstr /r "arduino-git:mbed.*9.9.9"
IF %ERRORLEVEL% NEQ 0 (
    GOTO INSTALLMBEDCORE
)

:AFTERINSTALLMBEDCORE
echo Finding Arduino MBED core OK

echo Finding Arduino MBED Nicla v3.1.1...

(arduino-cli core list  2> nul) | findstr /r "arduino:mbed_nicla.*3.1.1"
IF %ERRORLEVEL% NEQ 0 (
    echo Installing Nicla board ...
    arduino-cli core update-index
    arduino-cli core install arduino:mbed_nicla@3.1.1
    echo Installing Nicla board OK
)

for /f "delims=" %%i in ('arduino-cli config dump ^| findstr /r "data: "') do (
    set OUTPUT=%%i
)
IF "%OUTPUT%" == "" GOTO LIBRARYDIRNOTFOUND

echo Checking NDP_utils 1.0.2 ...
(arduino-cli lib list NDP_utils 2> nul) | findstr /r "NDP_utils 1.0.2"
IF %ERRORLEVEL% NEQ 0 (
    arduino-cli lib uninstall NDP_utils
    echo Installing NDP_utils library...
    mkdir "%library%\NDP_utils"
    xcopy "lib\NDP_utils" "%library%\NDP_utils" /E /V /Y
    echo Installing NDP_utils library OK
)

echo Checking Nicla_System 1.0 ...
rem for now let's name it 1.2 ....
(arduino-cli lib list Nicla_System 2> nul) | findstr /r "Nicla_System 1.0"
IF %ERRORLEVEL% NEQ 0 (
    arduino-cli lib uninstall Nicla_System
    echo Installing Nicla_System library...
    mkdir "%library%\Nicla_System"
    xcopy "lib\Nicla_System" "%library%\Nicla_System" /E /V /Y
    echo Installing Nicla_System library OK
)

echo Checking syntiant_ilib 1.2 ...
rem for now let's name it 1.2 ....
(arduino-cli lib list syntiant_ilib 2> nul) | findstr /r "syntiant_ilib 1.2"
IF %ERRORLEVEL% NEQ 0 (
    arduino-cli lib uninstall syntiant_ilib
    echo Installing syntiant_ilib library...
    mkdir "%library%\syntiant_ilib"
    xcopy "lib\syntiant_ilib" "%library%\syntiant_ilib" /E /V /Y
    echo Installing syntiant_ilib library OK
)

REM TODO install nicla system

echo Installing of libraries completed ...
@pause
exit /b 0

:NOTINPATHERROR
echo Cannot find 'arduino-cli' in your PATH. Install the Arduino CLI before you continue
echo Installation instructions: https://arduino.github.io/arduino-cli/latest/
@pause
exit /b 1

:INSTALLMBEDCORE
echo Installing Arduino MBED core...
REM Lib has not yet been released, you should already have it installed! Throwing an error for now
echo Downloading mbed experimental release - be patient it may take a while...
powershell -c "Invoke-WebRequest -Uri ' https://cdn.edgeimpulse.com/build-system/hardware-mbed-git-nicla-voice-v91-imu.zip' -OutFile 'hardware-mbed-git-nicla-syntiant.zip'"
powershell -c "tar -xf hardware-mbed-git-nicla-syntiant.zip -C '%arduinodirectory%'"
del hardware-mbed-git-nicla-syntiant.zip
echo mbded core v9.9.9 installed
GOTO AFTERINSTALLMBEDCORE

:UPGRADECLI
echo You need to upgrade your Arduino CLI version (now: %CLI_MAJOR%.%CLI_MINOR%.%CLI_REV%, but required: %EXPECTED_CLI_MAJOR%.%EXPECTED_CLI_MINOR%.x or higher)
echo See https://arduino.github.io/arduino-cli/installation/ for upgrade instructions
@pause
exit /b 1

:LIBRARYDIRNOTFOUND
echo Arduino library directory not found. 
@pause
exit /b 1

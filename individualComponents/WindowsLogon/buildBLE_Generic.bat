@echo off
echo Setting global Variables for Generic Build

if not defined MSBUILD_BITNESS (
	echo Bitness was not defined, defaulting to 64 bit
	set MSBUILD_BITNESS=/p:Platform=x64
	set MSBUILD_BITNESSC=/p:Platform=x64
)
set RADIUS_RELATIVE_DIR= %~dp0/../RADIUS/

REM set MSBUILD_GLOBAL_ARGS=/p:configuration=Debug %MSBUILD_BITNESS%
REM set MSBUILD_GLOBAL_ARGSC=/p:configuration=Debug %MSBUILD_BITNESSC%
set MSBUILD_GLOBAL_ARGS=/p:configuration=Release %MSBUILD_BITNESS%
set MSBUILD_GLOBAL_ARGSC=/p:configuration=Release %MSBUILD_BITNESSC%

echo Building Mi-Token BLE Login Generic
echo %~dp0

cd /d %~dp0



REM Build the BLE Sample code (will build the BLELibV2 / BLEDLLV2 / ConfigTool / etc)
cd ./BLESample
call Build.bat
if ERRORLEVEL 1 goto :failed

set MSBUILD_PATH=%SystemRoot%\microsoft.net\Framework\v4.0.30319\msbuild.exe
set MSBUILD_ARGS=
REM Build everything else needed for the BLE Login (OTPCheck/MiTokenOTPLib/MiTokenBLELoginASync)

REM TODO : Fix this, but for the moment override MSBUILD_GLOBAL_ARGS to have the correct configuration
REM set MSBUILD_GLOBAL_ARGS=/p:configuration=Release /p:Platform=x64

REM MiTokenAPICpp
cd /d %~dp0
cd ./Gina/MiTokenAPICpp
%MSBUILD_PATH% MiTokenAPICpp.vcxproj %MSBUILD_ARGS% %MSBUILD_GLOBAL_ARGS%
if ERRORLEVEL 1 goto :failed

REM MiTokenOTPLib
cd /d %RADIUS_RELATIVE_DIR%
cd ./MiTokenOTPLib
%MSBUILD_PATH% MiTokenOTPLib.vcxproj %MSBUILD_ARGS% %MSBUILD_GLOBAL_ARGS%
if ERRORLEVEL 1 goto :failed

REM OTPCheck
cd /d %~dp0
cd ./Gina/OTPCheck
echo BUILDING : %MSBUILD_PATH% OTPCheck.vcxproj %MSBUILD_ARGS% %MSBUILD_GLOBAL_ARGS%
%MSBUILD_PATH% OTPCheck.vcxproj %MSBUILD_ARGS% %MSBUILD_GLOBAL_ARGS%
if ERRORLEVEL 1 goto :failed

REM MiTokenBLELoginASync
cd /d %~dp0
cd ./MiTokenBLELoginAsync/MiTokenBLELoginAsync
%MSBUILD_PATH% MiTokenBLELoginAsync.vcxproj %MSBUILD_ARGS% %MSBUILD_GLOBAL_ARGS%
if ERRORLEVEL 1 goto :failed

REM Build the BLE Login Installer
cd /d %~dp0
cd ./Gina/BLEInstaller
%MSBUILD_PATH% BLEInstaller.wixproj %MSBUILD_ARGS% %MSBUILD_GLOBAL_ARGSC%
if ERRORLEVEL 1 goto :failed


echo Generic Build Completed
cd /d %~dp0
goto :eof


:failed
cd /d %~dp0
echo An error occured during the building
pause
exit /b 1

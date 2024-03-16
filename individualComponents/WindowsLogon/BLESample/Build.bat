REM @echo off
echo Building Mi-Token BLE Code 64bit

if not defined MSBUILD_GLOBAL_ARGS set MSBUILD_GLOBAL_ARGS=

echo Global args = %MSBUILD_GLOBAL_ARGS%
echo %~dp0

if not defined MSBUILD_PATH set MSBUILD_PATH=%SystemRoot%\microsoft.net\Framework\v4.0.30319\msbuild.exe

set MSBUILD_ARGS=/p:SolutionDir=%~dp0

REM Build the Library file first - basically everything uses it
cd /d %~dp0
cd ./BLELibV2
echo Building BLE Lib V2
%MSBUILD_PATH% BLELibV2.vcxproj %MSBUILD_ARGS% %MSBUILD_GLOBAL_ARGS%

if ERRORLEVEL 1 exit /b 1

REM Build the BLE_BondingLib now - it needs the Library and is used by the DLL
cd /d %~dp0
cd ./BLE_BondingLib
echo Building BLE Bonding Lib
%MSBUILD_PATH% BLE_BondingLib.vcxproj %MSBUILD_ARGS% %MSBUILD_GLOBAL_ARGS%

if ERRORLEVEL 1 exit /b 1


REM Build the DLL second - it needs the main Library and the BLE_BondingLib library
cd /d %~dp0
cd ./BLEDLLV2
echo Building BLE DLL V2
%MSBUILD_PATH% BLEDLLV2.vcxproj %MSBUILD_ARGS% %MSBUILD_GLOBAL_ARGS%
if ERRORLEVEL 1 exit /b 1

REM Build the CPTokenClass Library
cd /d %~dp0
cd ./CPTokenClass
echo Building CP Token Class Lib
%MSBUILD_PATH% CPTokenClass.vcxproj %MSBUILD_ARGS% %MSBUILD_GLOBAL_ARGS%
if ERRORLEVEL 1 exit /b 1

REM Build the configuration tool
cd /d %~dp0
cd "./BLE Configuration Tool"
echo Building Configuration Tool
%MSBUILD_PATH% "BLE Configuration Tool.csproj" %MSBUILD_ARGS% %MSBUILD_GLOBAL_ARGSC%
if ERRORLEVEL 1 exit /b 1

REM Build the BLELockout tool
cd /d %~dp0
cd "./BLELockout"
echo Building BLE Lockout
%MSBUILD_PATH% "BLELockout.vcxproj" %MSBUILD_ARGS% %MSBUILD_GLOBAL_ARGS%
if ERRORLEVEL 1 exit /b 1

REM Build the new BLEWatcher Service
cd /d %~dp0
cd "./MiTokenBLEWatcher"
echo Building MiTokenBLEWatcher
%MSBUILD_PATH% "MiTokenBLEWatcher.csproj" %MSBUILD_ARGS% %MSBUILD_GLOBAL_ARGSC%
if ERRORLEVEL 1 exit /b 1

REM Build the BLEWatcher service
REM cd /d %~dp0
REM cd "./Mi-Token BLE Watcher"
REM echo Building Mi-Token BLE Watcher Service
REM %MSBUILD_PATH% "Mi-Token BLE Watcher.csproj" %MSBUILD_ARGS% %MSBUILD_GLOBAL_ARGSC%
REM if ERRORLEVEL 1 exit /b 1

REM Build the Provisioning Tool Installer
cd /d %~dp0
cd "./BLEProvisioningToolInstaller"
echo Building the Provisioning Tool Installer
%MSBUILD_PATH% "BLEProvisioningToolInstaller.wixproj" %MSBUILD_ARGS% %MSBUILD_GLOBAL_ARGSC%
if ERRORLEVEL 1 exit /b 1

echo BLE Library Code Built
exit /b 0
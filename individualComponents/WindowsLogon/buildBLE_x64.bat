@echo off
echo Building 64bit BLE Solution

set MSBUILD_BITNESS=/p:Platform=x64 /p:PlatformTarget=x64
set MSBUILD_BITNESSC=/p:Platform=x64 /p:PlatformTarget=x64
cd /d %~dp0
call buildBLE_Generic.bat
if ERRORLEVEL 1 goto :failed

echo 64Bit BLE solution built
cd /d %~dp0
exit /b 0

:failed
cd /d %~dp0
echo 64Bit BLE solution failed
exit /b 1

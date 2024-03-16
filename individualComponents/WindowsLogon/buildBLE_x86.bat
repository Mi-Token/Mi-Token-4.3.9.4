@echo off
echo Building 32bit BLE Solution

set MSBUILD_BITNESS=/p:Platform=Win32 /p:PlatformTarget=x86
set MSBUILD_BITNESSC=/p:Platform=x86 /p:PlatformTarget=x86
cd /d %~dp0
call buildBLE_Generic.bat
if ERRORLEVEL 1 goto :failed

echo 32Bit BLE solution built
cd /d %~dp0
exit /b 0

:failed
cd /d %~dp0
echo 32Bit BLE solution failed
exit /b 1

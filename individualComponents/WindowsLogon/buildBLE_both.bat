@echo off
cd /d %~dp0

call buildBLE_x64.bat
if ERRORLEVEL 1 goto :failed

call buildBLE_x86.bat
if ERRORLEVEL 1 goto :failed

echo Build completed
pause
exit /b 0

:failed
echo Build failed
pause
exit /b 1
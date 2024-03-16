@echo off
echo Building 64bit CP Solution Standalone

echo Setting up variables to mimic how it works with RADIUS script


echo Running scripts from RADIUS build
cd /d %~dp0\..\RADIUS\
echo Running Version.js
cscript Version.js
echo Running buildprep.py
python buildprep.py

cd /d %~dp0

set MSBUILD_PATH=%SystemRoot%\microsoft.net\Framework\v4.0.30319\msbuild.exe
set MITOKEN_MSBUILD_LOGGERS=/noconsolelogger /l:FileLogger,Microsoft.Build.Engine;LogFile=BUILD.%1bit.msbuild.log;Append=true;Verbosity=diagnostic /l:ConsoleLogger,Microsoft.Build.Engine;performancesummary

echo Running normal buildCP_x64 batch file
call buildCP_x64.bat 64
if ERRORLEVEL 1 goto :failed

echo Build completed successfully
exit /b 0

:failed
echo Build failed
exit /b 1


@echo off
echo Building CP Cache Project

echo Removing build output
rmdir "Build Cache" /S /Q

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


echo Copying over versioning
xcopy "..\RADIUS\Version.cs" "Version.cs" /Y
xcopy "..\RADIUS\MiTokenIAS\version.h" "Version.h" /Y


echo Doing Pre-builds
%MSBUILD_PATH% %MITOKEN_MSBUILD_LOGGERS% ..\RADIUS\build\MSBuild.Community.Tasks\MSBuild.Community.Tasks\MSBuild.Community.Tasks.csproj 
%MSBUILD_PATH% %MITOKEN_MSBUILD_LOGGERS% ..\RADIUS\build\MiToken.Build.CustomTasks\MiToken.Build.CustomTasks.csproj
%MSBUILD_PATH% %MITOKEN_MSBUILD_LOGGERS% ..\RADIUS\build\MSBuild.Mercurial\MSBuild.Mercurial.csproj

echo Building projects
%MSBUILD_PATH% %MITOKEN_MSBUILD_LOGGERS% CPCache.msbuild.xml  /maxcpucount:1 /property:MiToken_Output_Bitness=64

echo Build completed successfully
exit /b 0

:failed
echo Build failed
exit /b 1


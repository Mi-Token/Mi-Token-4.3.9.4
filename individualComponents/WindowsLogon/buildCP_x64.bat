@echo off

cd /d %~dp0

echo Copying over versioning
xcopy "..\RADIUS\Version.cs" "Version.cs" /Y
xcopy "..\RADIUS\MiTokenIAS\version.h" "Version.h" /Y

echo Building projects
%MSBUILD_PATH% %MITOKEN_MSBUILD_LOGGERS% CP.msbuild.xml  /maxcpucount:1 /property:MiToken_Output_Bitness=%1

exit /b 0
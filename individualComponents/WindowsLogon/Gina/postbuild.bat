cd %~dp0\Release
MetaInstaller.exe -x86 "\..\..\build\x86\Release\InstallBootstrapper.exe" -x64 "\..\..\build\x64\Release\InstallBootstrapper.exe" -o "\PackedInstaller.exe" -FileType BOOT
rem pause
exit
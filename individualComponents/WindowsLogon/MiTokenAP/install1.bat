xcopy .\Release\MiTokenAP.dll "C:\Windows\System32\MiTokenAP.dll" /Y
if %errorlevel% neq 0 (
	echo "FAILED - Trying Install 2.bat"
	install2.bat
	exit /b %errorlevel%
)
Reg1.reg
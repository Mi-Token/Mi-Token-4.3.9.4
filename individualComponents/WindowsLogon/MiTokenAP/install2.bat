xcopy .\Release\MiTokenAP.dll "C:\Windows\System32\MiTokenAP2.dll" /Y
if %errorlevel% neq 0 exit /b %errorlevel%

Reg2.reg
echo Writing Registry Keys:
Reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers\{CB0D3EB9-2E00-44BC-8EC0-221A984E42B6}" /t REG_SZ /d "MiTokenCredProvider" /f
Reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Provider Filters\{CB0D3EB9-2E00-44BC-8EC0-221A984E42B6}" /t REG_SZ /d "MiTokenCredProvider" /f
Reg add "HKCR\CLSID\{CB0D3EB9-2E00-44BC-8EC0-221A984E42B6}" /t REG_SZ /d "MiTokenCredProvider" /f
Reg add "HKCR\CLSID\{CB0D3EB9-2E00-44BC-8EC0-221A984E42B6}\InprocServer32" /t REG_SZ /d "MiTokenCredProvider.dll" /f
Reg add "HKCR\CLSID\{CB0D3EB9-2E00-44BC-8EC0-221A984E42B6}\InprocServer32" /v "ThreadingModel" /t REG_SZ /d "Apartment" /f
echo Copying DLL to System32
COPY MiTokenCredProviderX64.dll "C:\Windows\System32\MiTokenCredProvider.dll" /Y 
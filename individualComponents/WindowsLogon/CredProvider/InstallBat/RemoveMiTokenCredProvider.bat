echo Removing Registry Keys:
Reg delete "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers\{CB0D3EB9-2E00-44BC-8EC0-221A984E42B6}" /f
Reg delete "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Provider Filters\{CB0D3EB9-2E00-44BC-8EC0-221A984E42B6}" /f
Reg delete "HKCR\CLSID\{CB0D3EB9-2E00-44BC-8EC0-221A984E42B6}" /f
Reg delete "HKCR\CLSID\{CB0D3EB9-2E00-44BC-8EC0-221A984E42B6}\InprocServer32" /v "ThreadingModel" /f
Reg delete "HKCR\CLSID\{CB0D3EB9-2E00-44BC-8EC0-221A984E42B6}\InprocServer32" /f
echo Copying DLL to System32
XCOPY MiTokenCredProviderX32.dll "C:\Windows\System32\MiTokenCredProvider.dll" /Q /Y  
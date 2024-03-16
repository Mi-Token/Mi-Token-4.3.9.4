function Register

	Dim WshShell, ginaPath
	Set WshShell = CreateObject("WScript.Shell")
	ginaPath = "c:\Program Files\Mi-Token\Mi-Token GINA\MiTokenGINA.dll"
	WshShell. RegWrite "HKLM\Software\Microsoft\Windows NT\CurrentVersion\Winlogon\GinaDLL", ginaPath, "REG_SZ"
	
end function

function Unregister

	Dim WshShell, ginaPath
	Set WshShell = CreateObject("WScript.Shell")
	ginaPath = "c:\windows\system32\msgina.dll"
	WshShell. RegWrite "HKLM\Software\Microsoft\Windows NT\CurrentVersion\Winlogon\GinaDLL", ginaPath, "REG_SZ"
	
end function

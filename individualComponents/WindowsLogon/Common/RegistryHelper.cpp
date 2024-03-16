#include <exception>
#include <sstream>

#include "RegistryHelper.h"

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

std::vector<BYTE> RegistryHelper::ReadSingleStringValue(CString keyPath, CString valueName)
{
	return ReadValue(keyPath, valueName, REG_SZ);
}


std::vector<BYTE> RegistryHelper::ReadBinaryValue(CString keyPath, CString valueName)
{
	return ReadValue(keyPath, valueName, REG_BINARY);
}

std::vector<std::vector<BYTE>> RegistryHelper::ReadMultiStringValue(CString keyPath, CString valueName)
{
	std::vector<BYTE> values =  ReadValue(keyPath, valueName, REG_MULTI_SZ/*Specifies an array of null-terminated strings, terminated by two null characters.*/);

	std::vector<std::vector<BYTE>> separatedValues;
	std::vector<BYTE> *value = new std::vector<BYTE>();

	int byteCount = 0;
	for (std::vector<BYTE>::iterator itr = values.begin() ; itr < values.end(); itr++ )
	{	if(*itr == '\0')
		{
			value->push_back(*itr);
			++byteCount;
			if(byteCount == 3)
			{
				byteCount = 0;
				separatedValues.push_back(*value);
				value = new std::vector<BYTE>();
			}
		}
		else
		{
			byteCount = 0;
			value->push_back(*itr);
		}
	}

	return separatedValues;
}

//std::vector<BYTE> RegistryHelper::ReadBinaryValue(CString keyPath, CString valueName)
//{
//	return ReadValue(keyPath, valueName, REG_BINARY);
//}

std::vector<BYTE> RegistryHelper::ReadValue(CString keyPath, CString valueName, DWORD type)
{
	HKEY hKey = NULL;
	DWORD length = 0;

	try
	{
		LONG result = RegOpenKeyEx (HKEY_LOCAL_MACHINE,keyPath,0,KEY_READ,&hKey);

		if(result == ERROR_SUCCESS)
		{
			/*Get the size of the registry value.*/
			result = RegQueryValueEx(hKey, valueName, NULL, &type, NULL, &length);

			if(result != ERROR_SUCCESS)
				ThrowCustomException(result);

			std::vector<BYTE> regVal(length);

			result = RegQueryValueEx(hKey, valueName, NULL, &type, &regVal[0], &length);

			if(result == ERROR_SUCCESS)
			{
				/*Succes*/
				RegCloseKey(hKey);
				return regVal;
			}
		}

		ThrowCustomException(result);
	}
	catch(const std::exception &ex)
	{
		if(hKey != NULL)
			RegCloseKey(hKey);

		throw;
	}
}

/*Throw custom exception so that you can ignore if necessary.*/
void RegistryHelper::ThrowCustomException(DWORD retcode)
{
	if (retcode == ERROR_FILE_NOT_FOUND) 
		throw NameValueException();
	else 
		/*TODO  call FormatMessage to format the message.*/
		throw new RegistryHelperException("Error while opening name value. Error code");
}

std::vector<RadiusServer> RegistryHelper::RetrieveRadiusServers(CString keyPath)
{
	std::vector<RadiusServer> availableServers;
	HKEY hKey = NULL;
	DWORD    cValues;              // number of values for key 
	DWORD    cchMaxValue;          // longest value name 
	DWORD    cbMaxValueData;       // longest value data 

	DWORD retCode; 

	TCHAR  achValue[MAX_VALUE_NAME]; 
	DWORD cchValue = MAX_VALUE_NAME; 
	CString valueName;
	LONG result = RegOpenKeyEx (HKEY_LOCAL_MACHINE,keyPath,0,KEY_READ,&hKey);

	if(result == ERROR_SUCCESS)
	{
		// Get the class name and the value count. 
		retCode = RegQueryInfoKey(
			hKey,                    // key handle 
			NULL,                    // buffer for class name 
			NULL,                    // size of class string 
			NULL,                    // reserved 
			NULL,                    // number of subkeys 
			NULL,                    // longest subkey size 
			NULL,                    // longest class string 
			&cValues,                // number of values for this key 
			&cchMaxValue,            // longest value name 
			&cbMaxValueData,         // longest value data 
			NULL,                    // security descriptor 
			NULL);					 // last write time 

		if (cValues) 
		{
			for (DWORD i = 0; i < cValues; i++) 
			{ 
				/*Reset the buffer*/
				cchValue = MAX_VALUE_NAME; 
				memset (achValue,'\0',MAX_VALUE_NAME * sizeof(TCHAR));
				//achValue[0] = '\0'; 

				retCode = RegEnumValue(
					hKey, 
					i,			//The index of the value to be retrieved.
					achValue,   //A pointer to a buffer that receives the name of the value as a null-terminated string.
					&cchValue,  //A pointer to a variable that specifies the size of the buffer 
					NULL,		//This parameter is reserved and must be NULL.
					NULL,		//A pointer to a variable that receives a code indicating the type of data stored in the specified value.
					NULL,		//A pointer to a buffer that receives the data for the value entry.
					NULL		//A pointer to a variable that specifies the size of the buffer 
					);

				if(retCode == ERROR_SUCCESS)
				{
					valueName = achValue;
					/*Continue processing if the name value is a Raduis Server.*/
					if(valueName.Find(L"Server_Name_") > -1)
					{
						try
						{

							/*If port and shared secret is not avaible ignore the server.*/
							RadiusServer *server = new RadiusServer();
							std::vector<BYTE> value = ReadValue(keyPath,valueName,REG_SZ);
							server->serverName = reinterpret_cast< TCHAR *>(&value[0]);

							/*Get the server number.*/
							valueName.Replace(L"Server_Name_", L"");

							/*Server port.*/
							value = ReadValue(keyPath,L"Server_Port_" + valueName, REG_SZ);
							server->serverPort = _wtoi(reinterpret_cast< TCHAR *>(&value[0]));

							/*Shared secret*/
							value = ReadValue(keyPath,L"Server_Secret_" + valueName, REG_SZ);

							//value = ReadValue(keyPath,L"Server_Secret_" + valueName, REG_BINARY);

							server->serverSecret = reinterpret_cast< TCHAR *>(&value[0]);
							CString cs(( const char * )&value[0] );


							availableServers.push_back(*server);
						}
						catch(std::exception &ex)
						{
							//*Need to log the errors.*/
						}
					}
				}
			}
		}
	}
	return availableServers;

}


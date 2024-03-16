#include <atlstr.h>
#include <exception>
#include <vector>
#include <list>

#ifndef	REGISTRY_HEADER_H
#define REGISTRY_HEADER_H

class RadiusServer;
class RegistryHelper
{
	public:
		static std::vector<BYTE> ReadSingleStringValue(CString keyPath, CString valueName);
		static std::vector<std::vector<BYTE>> ReadMultiStringValue(CString keyPath, CString valueName);
		static std::vector<BYTE> ReadBinaryValue(CString keyPath, CString valueName);
		static std::vector<RadiusServer> RetrieveRadiusServers(CString keyPath);
	private:
		static std::vector<BYTE> ReadValue(CString keyPath, CString valueName, DWORD Type);
		static void ThrowCustomException(DWORD retCode);

};

class RegistryHelperException: public std::exception
{
public :
	RegistryHelperException( const char* why): exception(why)
	{
	}
};

class KeyMissingException: public std::exception
{
  virtual const char* what() const throw()
  {
    return "Key missing!!!";
  }
};

class NameValueException: public std::exception
{
  public:
	NameValueException(CString missing = L"Not Specified"):missingNameValue(missing)
    {  
    }

  virtual const char* what() const throw()
  {
	CT2CA ansiString (missingNameValue);
	std::string stdString (ansiString);
	stdString.append(" : Name Value missing!!!");
	return stdString.c_str();
  }
  private:
	CString missingNameValue;
};

class RadiusServer
{
	public:
		CString serverName;
		INT serverPort;
		CString serverSecret;
};

#endif /* REGISTRY_HEADER_H */

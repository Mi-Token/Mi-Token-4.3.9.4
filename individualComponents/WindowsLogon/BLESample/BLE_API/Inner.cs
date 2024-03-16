using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BLE_API
{
    public partial class BLE
    {
        internal class Inner
        {
            internal static Dictionary<IntPtr, BLE.V2_1.MiTokenBLE> interfaceDictionary = new Dictionary<IntPtr, BLE.V2_1.MiTokenBLE>();
            internal static Dictionary<IntPtr, BLE.V2_2.MiTokenBLE> interfaceDictionaryV2_2 = new Dictionary<IntPtr, BLE.V2_2.MiTokenBLE>();

            #region Disallow Exception Throw Functions
            internal static void DLLV1Allowed()
            {
                if(Settings.Disallow_DLLV1)
                {
                    throw new Exceptions.DllVersionDisallowedException("Using DLLV1 Functions are disallowed");
                }
            }

            internal static void DLLV2Allowed()
            {
                if (Settings.Disallow_DLLV2)
                {
                    throw new Exceptions.DllVersionDisallowedException("Using DLLV2 Functions are disallowed");
                }
            }

            internal static void DLLV2_1_Allowed()
            {
                if (Settings.Disallow_DLLV2_1)
                {
                    throw new Exceptions.DllVersionDisallowedException("Using DLL V2.1 Functions are disallowed");
                }
            }

            internal static void DLLV2_2_Allowed()
            {
                if (Settings.Disallow_DLLV2_2)
                {
                    throw new Exceptions.DllVersionDisallowedException("Using DLL V2.2 Functions are disallowed");
                }
            }

            #endregion

            #region DLL Version Stuff
            internal static int Version(int major, int minor, int revision)
            {
                return (major << 16) | (minor << 8) | (revision);
            }

            internal static void ReverseVersion(int versionNumber, out int major, out int minor, out int revision)
            {
                major = versionNumber >> 16;
                minor = ((versionNumber >> 8) & 0xFF);
                revision = (versionNumber & 0xFF);
            }

            internal static int DLL_Wrapper_Version = Version(2, 2, 3);
            internal static int DLL_Version = Version(2, 0, 0);
            internal static bool checkedDLLVersion = false;
            internal static bool validatedWrapper = false;

            internal static bool VerifyDllVersion(int minimumVersion) //will always throw errors on failures and validate the wrapper
            {
                if (!checkedDLLVersion)
                {
                    try
                    {
                        DLL_Version = DLLFunctions.Inner.DLL_GetVersion();
                        checkedDLLVersion = true;
                    }
                    catch (Exception ie)
                    {
                        throw new Exceptions.DLLVersionCheckFailed("DLL Version check failed while trying to get the DLL Version", ie);
                    }
                }

                if (!validatedWrapper)
                {
                    try
                    {
                        if (DLLFunctions.Inner.DLL_CheckWrapperVersion(DLL_Wrapper_Version) != 0)
                        {
                            throw new Exceptions.DLLVersionCheckFailed("DLL Version check failed while verifying the wrapper version", null);
                        }
                        validatedWrapper = true;
                    }
                    catch (Exception ie)
                    {
                        throw new Exceptions.DLLVersionCheckFailed("DLL Version check failed while verifying the wrapper version", ie);
                    }
                }

                bool res = (DLL_Version >= minimumVersion);

                if (!res)
                {
                    throw new Exceptions.WrongDLLVersion(minimumVersion, DLL_Version);
                }

                return res;
                
            }
            #endregion
        }
    }
}


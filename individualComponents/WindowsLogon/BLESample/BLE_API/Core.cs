using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BLE_API
{
    public partial class BLE
    {
        public partial class Core
        {
            public const int Interface_Version_2_1 = 0x00020100;
            public const int Interface_Version_2_2 = 0x00020200;

            public const int CONN_TIMEOUT_INFINITE = -1;
            public const int CONN_TIMEOUT_JUST_CHECK = 0;

            static public int DLL_Version
            {
                get
                {
                    if (Inner.DLL_Version == Inner.Version(2, 0, 0))
                    {
                        Inner.VerifyDllVersion(Inner.Version(2, 0, 0));
                    }

                    return Inner.DLL_Version;
                }
            }

            static public int Wrapper_Version
            {
                get
                {
                    return Inner.DLL_Wrapper_Version;
                }
            }

            public enum API_RET
            {
                BLE_API_SUCCESS = 0,
                BLE_API_MORE_DATA = -1,
                BLE_API_ERR_NULL_POINTER = -2,
                BLE_API_ERR_NO_SUCH_DEVICE = -3,
                BLE_API_ERR_NO_SUCH_META_FLAG = -4,
                BLE_API_ERR_FAILED_GETTING_META_DATA = -5,
                BLE_API_ERR_FAILED_TO_OPEN_COM_PORT = -6,
                BLE_API_ERR_NULL_INTERFACE = -7,
                BLE_API_ERR_NULL_INTERNAL_INTERFACE = -8,
                BLE_API_ERR_INVALID_DEVICE_ID = -9,
                BLE_API_ERR_SEARCH_FINISHED = -10,
                BLE_API_ERR_INTERFACE_TO_BE_DELETED = -11,
                BLE_API_ERR_CLIENT_STILL_HAS_COM = -12,

                BLE_API_ERR_UNKNOWN = -255,
                BLE_API_ERR_FUNCTION_NOT_IMPLEMENTED = -256,
                BLE_API_ERR_DLL_FUNCTION_NOT_IMPLEMENTED = -257,
            };

            public enum CONN_RET
            {
                BLE_CONN_NEW_REQUEST = 0,
                BLE_CONN_SUCCESS = 1,
                BLE_CONN_BAD_REQUEST = 0,
                BLE_CONN_ERR_NOT_CONNECTED = -1,
                BLE_CONN_ERR_NO_SUCH_CONNECTION = -2,
                BLE_CONN_ERR_NO_SUCH_SERVICE = -3,
                BLE_CONN_ERR_SERVICES_NOT_SCANNED = -4,
                BLE_CONN_ERR_ATTRIBUTE_SCAN_IN_PROGRESS = -5,
                BLE_CONN_ERR_NO_SUCH_ATTRIBUTE = -6,
                BLE_CONN_ERR_LONG_WRITE_IN_PROGRESS = -7,
                BLE_CONN_ERR_ATTRIBUTE_BEING_READ = -8,
                BLE_CONN_ERR_ATTRIBUTE_HAS_MORE_DATA = -9,
                BLE_CONN_ERR_CONNECTION_TIMED_OUT = -10,

                BLE_CONN_ERR_NOT_IMPLEMENTED = -11,
                BLE_CONN_ERR_SYNC_TIMEOUT = -12,
                BLE_CONN_ERR_INTERNAL_ERROR = -13,
            };

            public enum API_NP_RET
            {
                BLE_API_NP_SUCCESS = CONN_RET.BLE_CONN_SUCCESS,

                BLE_API_NP_ERR_UNKNOWN = API_RET.BLE_API_ERR_UNKNOWN,
                BLE_API_NP_ERR_NULL_INTERNAL_INTERFACE = API_RET.BLE_API_ERR_NULL_INTERNAL_INTERFACE,

                BLE_API_EXCLUSIVE_ALLOWED = 1,
                BLE_API_EXCLUSIVE_ASKED = -20,
                BLE_API_EXCLUSIVE_REJECTED = -21,
                BLE_API_NP_ERR_NOT_CLIENT = -22,
                BLE_API_EXCLUSIVE_DISABLED = -23,
                BLE_API_NP_INTERNAL_ERROR = -24,
                BLE_API_NP_BAD_EXCLUSIVE_STATE = -25,
                BLE_API_NP_COM_FALLBACK = -26,
                BLE_API_NP_ERR_NOT_CONNECTED = -27,
            };

            public enum BOND_RET
            {
                BOND_RET_SUCCESS = 0,
                BOND_RET_FAILED,
                BOND_RET_ERR_TIMEOUT,
                BOND_RET_ERR_PREVBOND_MAXBOND,
            };

            public static BLE.V2_2.MiTokenBLE LookupOnIntPtrV2_2(IntPtr handle)
            {
                if (Inner.interfaceDictionaryV2_2.ContainsKey(handle))
                {
                    return Inner.interfaceDictionaryV2_2[handle];
                }
                else if (Inner.interfaceDictionary.ContainsKey(handle))
                {
                    return (BLE.V2_2.MiTokenBLE)Inner.interfaceDictionary[handle];
                }
                return null;
            }

            private static readonly sbyte MIN_RSSI_DBM = -89;
            private static readonly sbyte MAX_RSSI_DBM = -38;
                    
            public static int SignalQualityFromRSSI(sbyte sRSSI)
            {
                if((sRSSI == 0) || (sRSSI <= MIN_RSSI_DBM))
                {
                    return 0;
                }
                else if(sRSSI >= MAX_RSSI_DBM)
                {
                    return 100;
                }
                else
                {
                    return (((sRSSI - MIN_RSSI_DBM) * 100) / (MAX_RSSI_DBM - MIN_RSSI_DBM));
                }
            }

            public static string MacStringFromAddressBytes(byte[] address)
            {
                return (string.Format("{0:X2}:{1:X2}:{2:X2}:{3:X2}:{4:X2}:{5:X2}", address[0], address[1], address[2], address[3], address[4], address[5]));
            }
        }
    }
}

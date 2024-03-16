using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using OpenSSL.Crypto;

namespace BLE_Provisioning_Tool
{
    static public class Constants
    {
        static public readonly int DEVICE_ADDRESS_LEN = 6;
        static public readonly int DEVICE_UUID_LEN = 16;
        static public readonly int SYSTEM_ID_LEN = 8;
        static public readonly int DEVICE_UUID_STRING_LEN = (DEVICE_UUID_LEN * 2) + 4; // Since string is in hex and has deliminators
        static public readonly int COMPANY_ID_LEN = 4;
        static public readonly int NUM_ECC_DIGITS = 16;
        static public readonly int SHARED_SECRET_LENGTH = 20;//TODO MessageDigest.SHA1.Size;

        // System
        static public readonly string BASE_REGISTRY_PATH = "Software\\Mi-Token\\BLE\\Provision";
        static public readonly string SERIAL_PORT_REGISTER = "COMPort";
        static public readonly string DEV_NAME_FILTER_TYPE_REGISTER = "DevNameFilterType";
    }

    static public class GapAdvertDataTypes
    {
        static public readonly int GAP_ADTYPE_LOCAL_NAME_COMPLETE = 0x09; //!< Complete local name
        static public readonly int GAP_ADTYPE_MANUFACTURER_SPECIFIC = 0xFF; //!< Manufacturer Specific Data: first 2 octets contain the Company Identifier Code followed by the additional manufacturer specific data
    }

    enum AppErrorCodes { 
      // System Configuration Service
      ATT_ERR_SYSCFG_FAIL = 0x480 + 0x1,
      ATT_ERR_SYSCFG_CMD_FAIL,
      ATT_ERR_SYSCFG_ENCRCMD_FAIL,
  
      // Secure Bond Service
      ATT_ERR_SECUREBOND_SMSTATUS_FAIL,
      ATT_ERR_SECUREBOND_CFG_ADD_FAIL,
      ATT_ERR_SECUREBOND_CFG_RMV_FAIL,
      ATT_ERR_SECUREBOND_CFG_ERASEALL_FAIL,
      ATT_ERR_SECUREBOND_ENCRCFG_FAIL,
      ATT_ERR_SECUREBOND_ENCRCFG_RMV_FAIL,
      ATT_ERR_SECUREBOND_ENCRCFG_UPDATEPIN_FAIL,
      ATT_ERR_SECUREBOND_ENCRCFG_UNINIT_FAIL,
  
      // Authentication Service
      ATT_ERR_AUTH_FAIL,
      ATT_ERR_AUTH_NOBOND_FAIL,
      ATT_ERR_AUTH_MAXRETRIES_FAIL,
      ATT_ERR_AUTH_INCORRECTPIN_FAIL,
  
      // User Key Service
      ATT_ERR_USERKEY_FAIL,
      ATT_ERR_USERKEY_ADD_FAIL,
      ATT_ERR_USERKEY_RMV_FAIL,
      ATT_ERR_USERKEY_GETDATA_FAIL,
      ATT_ERR_USERKEY_GETNAMES_FAIL,

      // Hash Service
      ATT_ERR_HASH_FAIL,
      ATT_ERR_HASH_BUFFEMPTY_FAIL,
      ATT_ERR_HASH_NOBOND_FAIL,
  
      // Secret Service
      ATT_ERR_SECRET_FAIL,
      ATT_ERR_SECRET_ADD_FAIL,
      ATT_ERR_SECRET_RMV_FAIL,
      ATT_ERR_SECRET_GETDATA_FAIL,
  
      // Oath Profile
      ATT_ERR_OATH_HOTP_FAIL,
      ATT_ERR_OATH_KEYSRC_FAIL,
  
      // Other Error Codes
      ATT_ERR_SYSCFG_ENCRCMD_BLOCKCOUNT_FAIL,
      ATT_ERR_SYSCFG_ENCRCMD_DECRYPT_FAIL,
      ATT_ERR_SECUREBOND_CMD_PING_FAIL,
      ATT_ERR_SECUREBOND_CMD_PING_V2_FAIL,
      ATT_ERR_SECUREBOND_CMD_CHECK_BOND_FAIL
    };
}

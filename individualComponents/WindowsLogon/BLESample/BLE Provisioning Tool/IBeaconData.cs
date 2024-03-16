using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace BLE_Provisioning_Tool
{
    public class IBeaconData
    {
        private class iBeaconData_t {
            public byte[] companyId = new byte[Constants.COMPANY_ID_LEN];
            public DeviceUUID deviceUuid = new DeviceUUID();
            public UInt16 major = 0;
            public UInt16 minor = 0;
            public byte measurePower = 0;

            public int SizeOf()
            {
                return companyId.Length + deviceUuid.Length + sizeof(UInt16) + sizeof(UInt16) + sizeof(byte);
            }
        }

        private iBeaconData_t m_data = new iBeaconData_t();
        private Boolean m_isInit = false;
        static readonly byte[] APPLE_COMANY_ID = new byte[] { 0x4c, 0x00, 0x02, 0x15 };

        public bool Init(ref byte[] inputArray)
        {
            if (inputArray.Length == m_data.SizeOf())
            {                
                byte[] companyId = inputArray.Skip(0).Take(Constants.COMPANY_ID_LEN).ToArray();
                if (companyId.SequenceEqual(APPLE_COMANY_ID))
                {
                    // Save Company ID
                    m_data.companyId = companyId;
                }
                else
                {
                    return false;
                }

                // Save Device UUID
                Array.Copy(inputArray.Skip(Constants.COMPANY_ID_LEN).ToArray(), m_data.deviceUuid.Value, m_data.deviceUuid.Length);

                // Save Major
                byte[] byteMajor = inputArray.Skip(Constants.COMPANY_ID_LEN + Constants.DEVICE_UUID_LEN).Take(sizeof(UInt16)).ToArray();
                m_data.major = (UInt16)BitConverter.ToInt16(byteMajor, 0);

                // Save Minor 
                byte[] byteMinor = inputArray.Skip(Constants.COMPANY_ID_LEN + Constants.DEVICE_UUID_LEN + sizeof(UInt16)).Take(sizeof(UInt16)).ToArray();
                m_data.minor = (UInt16)BitConverter.ToInt16(byteMinor, 0);
                    
                m_data.measurePower = inputArray.ElementAt(Constants.COMPANY_ID_LEN + Constants.DEVICE_UUID_LEN + sizeof(UInt16) + sizeof(UInt16));

                m_isInit = true;

                return true;
            }
            return false;
        }

        public DeviceUUID UUID
        {
            get
            {
                return m_data.deviceUuid;
            }

            set
            {
                m_data.deviceUuid = value;
            }
        }

        public UInt16 Major
        {
            get
            {
               return m_data.major;
            }

            set
            {
                m_data.major = value;
            }
        }

        public UInt16 Minor
        {
            get
            {
                return m_data.minor;
            }

            set
            {
                m_data.minor = value;
            }
        }

        public Boolean IsInit
        {
            get
            {
                return m_isInit;
            }
        }
    }
}

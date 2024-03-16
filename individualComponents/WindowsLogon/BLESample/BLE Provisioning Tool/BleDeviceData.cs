using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace BLE_Provisioning_Tool
{
    public class DeviceAddress
    {
        private byte[] m_address;

        public DeviceAddress()
        {
            m_address = new byte[Constants.DEVICE_ADDRESS_LEN];
        }

        public DeviceAddress(byte[] address)
        {
            m_address = new byte[Constants.DEVICE_ADDRESS_LEN];
            m_address = address;
        }

        public byte[] Value
        {
            get
            {
                return m_address;
            }

            set
            {
                if (value == null)
                {
                    throw new ArgumentNullException();
                }

                if (value.Length != Constants.DEVICE_ADDRESS_LEN)
                {
                    throw new ArgumentOutOfRangeException();
                }

                m_address = value;
            }
        }

        public int Length
        {
            get
            {
                return m_address.Length;
            }
        }

        public static bool operator ==(DeviceAddress x, DeviceAddress y)
        {
            return Helper.ArraysEqual(x.Value, y.Value);
        }

        public static bool operator !=(DeviceAddress x, DeviceAddress y)
        {
            return !Helper.ArraysEqual(x.Value, y.Value);
        }

        public override bool Equals(object o)
        {
            try
            {
                return (bool)(this == (DeviceAddress)o);
            }
            catch
            {
                return false;
            }
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        public string GetDisplayString()
        {
            byte[] reverseData = new byte[m_address.Length];
            reverseData = (byte[])m_address.Clone();
            Array.Reverse(reverseData);
            return GetString(reverseData);
        }

        public string GetRawString()
        {
            return GetString(m_address);
        }

        private string GetString(byte[] data)
        {
            int len = data.Length;
            if (len < Constants.DEVICE_ADDRESS_LEN)
                return String.Empty;

            StringBuilder hex = new StringBuilder(len * 2 + (len - 1));
            for (int i = 0; i < data.Length; ++i)
            {
                hex.AppendFormat("{0:X2}", data.ElementAt(i));
                if (i != data.Length - 1)
                {
                    hex.Append(":");
                }
            }
            return hex.ToString();
        }

        public string GetExportString()
        {
            // Make sure this is in little endian by reversing byte order
            byte[] reverseData = Helper.CreateDeviceID(this);
            Array.Reverse(reverseData);
            return Helper.GetExportHexString(reverseData);
        }

        public bool IsEmpty()
        {
            byte[] b = new byte[Constants.DEVICE_ADDRESS_LEN];
            for (int i = 0; i < b.Length; ++i)
            {
                b[i] = 0;
            }
            return Helper.ArraysEqual(this.Value, b);
        }
    }

    public class DeviceCompanyID
    {
        private byte[] m_companyID;

        public DeviceCompanyID()
        {
            m_companyID = new byte[Constants.COMPANY_ID_LEN];
        }

        public byte[] Value
        {
            get
            {
                return m_companyID;
            }

            set
            {
                if (value == null)
                {
                    throw new ArgumentNullException();
                }

                if (value.Length != Constants.COMPANY_ID_LEN)
                {
                    throw new ArgumentOutOfRangeException();
                }

                m_companyID = value;
            }
        }
    }

    public class DeviceUUID
    {
        private byte[] m_uuid;

        public DeviceUUID()
        {
            m_uuid = new byte[Constants.DEVICE_UUID_LEN];
        }

        public DeviceUUID(byte[] uuid)
        {
            m_uuid = new byte[Constants.DEVICE_UUID_LEN];
            m_uuid = uuid;
        }

        public byte[] Value
        {
            get
            {
                return m_uuid;
            }

            set
            {
                if (value == null)
                {
                    throw new ArgumentNullException();
                }

                if (value.Length != Constants.DEVICE_UUID_LEN)
                {
                    throw new ArgumentOutOfRangeException();
                }

                m_uuid = value;
            }
        }

        public int Length
        {
            get
            {
                return m_uuid.Length;
            }
        }

        public static bool operator ==(DeviceUUID x, DeviceUUID y)
        {
            return Helper.ArraysEqual(x.Value, y.Value);
        }

        public static bool operator !=(DeviceUUID x, DeviceUUID y)
        {
            return !Helper.ArraysEqual(x.Value, y.Value);
        }

        public override bool Equals(object o)
        {
            try
            {
                return (bool)(this == (DeviceUUID)o);
            }
            catch
            {
                return false;
            }
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        public string GetString()
        {
            int len = m_uuid.Length;

            if (len < Constants.DEVICE_UUID_LEN)
                return String.Empty;

            byte[] empty = new byte[len];
            if (Helper.ArraysEqual(m_uuid, empty))
            {
                return "<None>";
            }

            StringBuilder hex = new StringBuilder(len * 2 + 4);
            for (int i = 0; i < Constants.DEVICE_UUID_LEN; ++i)
            {
                hex.AppendFormat("{0:X2}", m_uuid.ElementAt(i));
                if ((i == 3) || (i == 5) || (i == 7) || (i == 9))
                {
                    hex.Append("-");
                }
            }
            return hex.ToString();
        }
    }

    public class BleDeviceData
    {
        // Properties
        public DeviceAddress Address { get; set; }
        public string DeviceName { get; set; }
        public BeaconData BeaconData { get; set; }
        public byte RSSI { get; set; }
        public bool IsBonded { get; private set; }
        public UInt32 LastPollSeen { get; private set; }
        public DateTime TimeOfLastPollSeen { get; set; }



        public Boolean IsSecureBondSvcExists { get; set; }
        public Boolean IsDevInfoSvcExists { get; set; }
        public Boolean IsSysCfgSvcExists { get; set; }
        public Boolean IsAuthenSvcExists { get; set; }
        public Boolean IsOadSvcExists { get; set; }
        public Boolean IsBuzzerExists { get; set; }
        public Boolean IsProvisioned { get; set; }
        public Boolean IsOadFirmware { get; set; }
        public ConfigurationData.eUseCase UseCase { get; set; }
        public Boolean IsAlreadyBonded { get; set; }


        // Constructors
        public BleDeviceData()
        {
            Address = new DeviceAddress();
            DeviceName = String.Empty;
            BeaconData = new BeaconData();
            RSSI = 0;   
            IsBonded = false;
            LastPollSeen = 0;

            IsSecureBondSvcExists = false;
            IsDevInfoSvcExists= false;
            IsSysCfgSvcExists= false;
            IsAuthenSvcExists= false;
            IsOadSvcExists= false;
            IsBuzzerExists= false;
            IsProvisioned= false;
            IsOadFirmware= false;
            UseCase = ConfigurationData.eUseCase.eInvalid;
            IsAlreadyBonded= false;    
        }

        public BleDeviceData(byte[] address, string name, BeaconData beaconData, byte rssi, UInt32 lastPollSeen, bool isBonded)
        {
            Address = new DeviceAddress(address);
            DeviceName = name;
            BeaconData = beaconData;
            RSSI = rssi;
            IsBonded = isBonded;
            LastPollSeen = lastPollSeen;
            TimeOfLastPollSeen = DateTime.Now;

            IsSecureBondSvcExists = false;
            IsDevInfoSvcExists = false;
            IsSysCfgSvcExists = false;
            IsAuthenSvcExists = false;
            IsOadSvcExists = false;
            IsBuzzerExists = false;
            IsProvisioned = false;
            IsOadFirmware = false;
            UseCase = ConfigurationData.eUseCase.eInvalid;
            IsAlreadyBonded = false;    
        }

        public BleDeviceData(DeviceAddress address)
        {
            Address = address;
            DeviceName = String.Empty;
            BeaconData = new BeaconData();
            RSSI = 0;
            IsBonded = false;
            LastPollSeen = 0;

            IsSecureBondSvcExists = false;
            IsDevInfoSvcExists = false;
            IsSysCfgSvcExists = false;
            IsAuthenSvcExists = false;
            IsOadSvcExists = false;
            IsBuzzerExists = false;
            IsProvisioned = false;
            IsOadFirmware = false;
            UseCase = ConfigurationData.eUseCase.eInvalid;
            IsAlreadyBonded = false;    
        }
    }
}

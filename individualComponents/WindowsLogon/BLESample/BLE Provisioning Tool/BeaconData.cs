using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace BLE_Provisioning_Tool
{
    public class BeaconData
    {
        // Constants
        static readonly byte[] APPLE_COMANY_ID = new byte[] { 0x4c, 0x00, 0x02, 0x15 };        

        // Properties
        public DeviceCompanyID CompanyID { get; private set; }
        public DeviceUUID UUID { get; set; }
        public UInt16 Major { get; set; }
        public UInt16 Minor { get; set; }        
        public byte MeasuredPower { get; private set; }
        public bool IsInit { get; private set; }

        // Constructors
        public BeaconData()
        {
            UUID = new DeviceUUID();
            CompanyID = new DeviceCompanyID();
            IsInit = false;
        }

        // Functions
        static private int ExpectedByteSize()
        {
            return Constants.COMPANY_ID_LEN + Constants.DEVICE_UUID_LEN + sizeof(UInt16) + sizeof(UInt16) + sizeof(byte);
        }

        public bool Init(ref byte[] inputArray)
        {
            if (inputArray.Length == ExpectedByteSize())
            {                
                byte[] companyId = inputArray.Skip(0).Take(Constants.COMPANY_ID_LEN).ToArray();
                if (companyId.SequenceEqual(APPLE_COMANY_ID))
                {
                    // Save Company ID
                    CompanyID.Value = companyId;
                }
                else
                {
                    return false;
                }

                // Save Device UUID
                Array.Copy(inputArray.Skip(Constants.COMPANY_ID_LEN).ToArray(), UUID.Value, UUID.Length);

                // Save Major
                byte[] byteMajor = inputArray.Skip(Constants.COMPANY_ID_LEN + Constants.DEVICE_UUID_LEN).Take(sizeof(UInt16)).ToArray();
                Array.Reverse(byteMajor); // Convert to little endian
                Major = (UInt16)BitConverter.ToInt16(byteMajor, 0);

                // Save Minor 
                byte[] byteMinor = inputArray.Skip(Constants.COMPANY_ID_LEN + Constants.DEVICE_UUID_LEN + sizeof(UInt16)).Take(sizeof(UInt16)).ToArray();
                Array.Reverse(byteMinor); // Convert to little endian
                Minor = (UInt16)BitConverter.ToInt16(byteMinor, 0);

                // Save Measured Power at 1m
                MeasuredPower = inputArray.ElementAt(Constants.COMPANY_ID_LEN + Constants.DEVICE_UUID_LEN + sizeof(UInt16) + sizeof(UInt16));

                IsInit = true;

                return true;
            }
            return false;
        }
    }
}

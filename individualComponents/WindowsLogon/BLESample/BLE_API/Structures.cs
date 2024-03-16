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
            public class Structures
            {
                public class DeviceInfo
                {
                    public byte[] macAddress;
                    public char RSSI;
                    public uint lastPollSeen;
                    public string macString()
                    {
                        if ((macAddress != null) && (macAddress.Length == 6))
                        {
                            return string.Format("{0:X2}:{1:X2}:{2:X2}:{3:X2}:{4:X2}:{5:X2}", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
                        }
                        return "";
                    }

                    public string MacString
                    {
                        get
                        {
                            if ((macAddress != null) && (macAddress.Length == 6))
                            {
                                return string.Format("{0:X2}:{1:X2}:{2:X2}:{3:X2}:{4:X2}:{5:X2}", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
                            }
                            return "";
                        }
                    }

                    public sbyte sRSSI
                    {
                        get
                        {
                            return (SByte)RSSI;
                        }
                    }

                    public int SignalQuality
                    {
                        get
                        {
                            return Core.SignalQualityFromRSSI(sRSSI);
                        }
                    }

                    public DeviceInfo()
                    {
                        macAddress = null;
                        RSSI = (char)0;
                        lastPollSeen = 0;
                    }

                    internal void FromBytes(byte[] data)
                    {
                        if ((macAddress == null) || (macAddress.Length != 6))
                        {
                            macAddress = new byte[6];
                        }

                        for (int i = 0; i < 6; ++i)
                        {
                            macAddress[i] = data[i];
                        }

                        RSSI = (char)data[6];

                        lastPollSeen = (uint)BitConverter.ToInt32(data, 7);

                    }
                }
            }
        }
    }
}

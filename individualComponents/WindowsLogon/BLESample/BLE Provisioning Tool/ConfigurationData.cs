using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace BLE_Provisioning_Tool
{
    public class ConfigurationData
    {
        public class Peripherals
        {
            public Peripherals()
            {
                Value = 0;
            }

            public Peripherals(UInt16 val)
            {
                Value = val;
            }

            public enum PeriID
            {
                LED_A = 0x0001,
                LED_B = 0x0002,
                LED_C = 0x0004,
                PUSHBUTTON_A = 0x0008,
                PUSHBUTTON_B = 0x0010,
                BUZZER = 0x0020,
                MISC_1 = 0x0040,
                MISC_2 = 0x0080,
                MISC_3 = 0x0100,
                MISC_4 = 0x0200,
            };

            public enum PeriType
            {
                LED,
                PUSHBUTTON,
                BUZZER,
                MISC_1,
                MISC_2,
                MISC_3,
                MISC_4,
                eLast // Last indicator
            };

            static Dictionary<PeriID, PeriType> m_peripheralTypeSet = new Dictionary<PeriID, PeriType>()
            {
                {PeriID.LED_A, PeriType.LED},
                {PeriID.LED_B, PeriType.LED},
                {PeriID.LED_C, PeriType.LED},
                {PeriID.PUSHBUTTON_A, PeriType.PUSHBUTTON},
                {PeriID.PUSHBUTTON_B, PeriType.PUSHBUTTON},
                {PeriID.BUZZER, PeriType.BUZZER},
                {PeriID.MISC_1, PeriType.MISC_1},
                {PeriID.MISC_2, PeriType.MISC_2},
                {PeriID.MISC_3, PeriType.MISC_3},
                {PeriID.MISC_4, PeriType.MISC_4},
            };

            static Dictionary<PeriType, String> m_peripheralNameSet = new Dictionary<PeriType, String>()
            {
                {PeriType.LED, "LED(s)"},
                {PeriType.PUSHBUTTON, "Pushbutton(s)"},
                {PeriType.BUZZER, "Buzzer"},
                {PeriType.MISC_1, ""},
                {PeriType.MISC_2, ""},
                {PeriType.MISC_3, ""},
                {PeriType.MISC_4, ""}
            };

            public UInt16 Value { get; set; }

            public bool IsExists(PeriID type)
            {
                return ((bool)(((UInt16)type & Value) != 0));
            }

            public String ToPrintString()
            {
                // Quickly return if no values found
                if (Value == 0)
                {
                    return "None";
                }

                // Counts number of same type peripherals using heap data structure
                uint[] heap = new uint[(int)PeriType.eLast];
                Array.Clear(heap, 0, heap.Length);
                foreach (KeyValuePair<PeriID, PeriType> entry in m_peripheralTypeSet)
                {
                    if (IsExists(entry.Key))
                    {
                        heap[(int)entry.Value]++;
                    }
                }

                // Creates string based on the various peripheral types
                StringBuilder sb = new StringBuilder();
                for (uint periType = 0; periType != heap.Length; ++periType)
                {
                    if (m_peripheralNameSet[(PeriType)periType] != "")
                    {
                        sb.AppendLine(String.Format("{0} x {1}", heap[periType], m_peripheralNameSet[(PeriType)periType]));
                    }
                }
                return sb.ToString();
            }
        };

        public interface IDeepCloneable
        {
            object DeepClone();
        }
        public interface IDeepCloneable<T> : IDeepCloneable
        {
            T DeepClone();
        }

        public class AdvSmStageSettings : IDeepCloneable<AdvSmStageSettings>
        {
            public enum Stages
            {
                Stage0 = 0,
                Stage1 = 1,
                Stage2 = 2,
                Stage3 = 3,
                StageEnd = 4
            };

            static public readonly int MIN_ADVSM_IDLETIMEOUT_MS = 1000; // 1 sec
            static public readonly int MAX_ADVSM_IDLETIMEOUT_MS = 86400000; // 24 hrs
            static public readonly int MIN_ADVSM_INTERVAL_MS = SysCfgService.ConvertAdvertIntervalToMs((UInt16)160); // 100 ms Note: Min=160(100ms)
            static public readonly int MAX_ADVSM_INTERVAL_MS = SysCfgService.ConvertAdvertIntervalToMs((UInt16)48000); // 1 min Note: Max=16000(10s)   

            private AdvertisingStateMachineStage_t[] _advSmSettings = new AdvertisingStateMachineStage_t[(int)Stages.StageEnd];

            public AdvSmStageSettings()
            {
            }

            public AdvSmStageSettings DeepClone()
            {
                // Deep clone your object
                AdvSmStageSettings clone = new AdvSmStageSettings();
                for (Stages stageId = Stages.Stage0; stageId != Stages.StageEnd; ++stageId)
                {
                    clone.Set(stageId, this.Get(stageId));
                }
                return clone;
            }

            object IDeepCloneable.DeepClone()
            {
                return this.DeepClone();
            }

            public byte[] Serialize()
            {
                List<byte> bb = new List<byte>();
                foreach (AdvertisingStateMachineStage_t settings in _advSmSettings)
                {
                    byte[] data = Helper.BuildByte(new byte[][] {
                        BitConverter.GetBytes(settings._idleTimeoutMs),
                        BitConverter.GetBytes(settings._intervalRaw)
                    });
                    foreach (byte b in data)
                    {
                        bb.Add(b);
                    }
                }
                
                return bb.ToArray();
            }

            public Boolean Deserialize(byte[] data)
            {
                if (data.Length != Size())
                {
                    return false;
                }

                int stageCount = 0;
                int idx = 0;
                while (idx < data.Length)
                {
                    _advSmSettings[stageCount]._idleTimeoutMs = BitConverter.ToUInt32(data, idx);
                    idx += sizeof(UInt32);
                    _advSmSettings[stageCount]._intervalRaw = BitConverter.ToUInt16(data, idx);
                    idx += sizeof(UInt16);
                    stageCount++;
                }
                return true;
            }

            public void Set(Stages stage, AdvertisingStateMachineStage_t settings)
            {
                _advSmSettings[(int)stage] = settings;
            }

            public void SetIdleTimeoutMs(Stages stage, UInt32 timeoutMs)
            {
                _advSmSettings[(int)stage]._idleTimeoutMs = timeoutMs;
            }

            public void SetIntervalMs(Stages stage, UInt16 intervalMs)
            {
                _advSmSettings[(int)stage]._intervalRaw = SysCfgService.ConvertAdvertIntervalToRaw(intervalMs);
            }

            public AdvertisingStateMachineStage_t Get(Stages stage)
            {
                return _advSmSettings[(int)stage];
            }

            public String GetIdleTimeoutMsStr(Stages stage)
            {
                return _advSmSettings[(int)stage]._idleTimeoutMs.ToString();
            }

            public String GetIntervalMsStr(Stages stage)
            {
                return SysCfgService.ConvertAdvertIntervalToMs(_advSmSettings[(int)stage]._intervalRaw).ToString();
            }

            static public int Size()
            {
                return Marshal.SizeOf(typeof(AdvertisingStateMachineStage_t)) * (int)Stages.StageEnd;
            }

            public Boolean IsEqual(AdvSmStageSettings compare)
            {                
                for (Stages stageId = Stages.Stage0; stageId != Stages.StageEnd; ++stageId)
                {
                    if ((this.Get(stageId)._idleTimeoutMs != compare.Get(stageId)._idleTimeoutMs) || (this.Get(stageId)._intervalRaw != compare.Get(stageId)._intervalRaw))
                    {
                        return false;
                    }
                }
                return true;
            }

            public String GetExportString()
            {
                StringBuilder bb = new StringBuilder();
                bb.Append("AdvertisingStages:" + Environment.NewLine);
                for (Stages stageId = Stages.Stage0; stageId != Stages.StageEnd; ++stageId)
                {
                    bb.AppendLine("\t" + stageId.ToString() + ": IdleTimeout(ms)=" + GetIdleTimeoutMsStr(stageId) + " Interval(ms)=" + GetIntervalMsStr(stageId));
                }
                bb.Append(Environment.NewLine);
                return bb.ToString();
            }
        }

        // Enumerations
        public enum eTxPowerLevel
        {
            eMinus23dBm = -23,
            eMinus6dBm = -6,
            e0dBm = 0,
            e4dBm = 4,
            eInvalid = 0xff
        };

        public enum eDeviceType
        {
            eCC2540,
            eCC2541,
            eInvalid
        };

        public enum eFormFactor
        {
            eVTAG,
            eF1,
            eMiniBeacon,
            eWellCore,
            eInvalid
        };

        public enum eUseCase
        {
            eBeacon,
            eAuthBeacon,
            eBlu,
            eBoot,
            eInvalid
        };

        public struct Revision_t
        {
            public int Major;
            public int Minor;
        };

        public struct MeasuredTxPower_t
        {
            public byte _Minus24dBm;
            public byte _Minus6dBm;
            public byte _0dBm;
            public byte _4dBm;
        };

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct AdvertisingStateMachineStage_t
        {
            public UInt32 _idleTimeoutMs;
            public UInt16 _intervalRaw;            
        };

        // Constants
        static public readonly int ALLOW_CFG_SET_REVISION_MAJOR = 1;
        static public readonly int ALLOW_CFG_SET_REVISION_MINOR = 1;
        static public readonly int MIN_DEV_NAME_LEN = 1;
        static public readonly int MAX_DEV_NAME_LEN = 20;
        static public readonly int MIN_ADVERT_INTERVAL_MS = 100;
        static public readonly int MAX_ADVERT_INTERVAL_MS = 60000;
        static public readonly int MIN_BATTERY_THRESHOLD_PERCENT = 0;
        static public readonly int MAX_BATTERY_THRESHOLD_PERCENT = 100;
        static public readonly int MIN_CONN_INTERVAL_MS = 8;
        static public readonly int MAX_CONN_INTERVAL_MS = 4000;
        static public readonly int MIN_CONN_LATENCY = 0;
        static public readonly int MAX_CONN_LATENCY = 500;
        static public readonly int MIN_CONN_TIMEOUT_MS = 100;
        static public readonly int MAX_CONN_TIMEOUT_MS = 32000;
        static public readonly int MIN_MAJOR = 0;
        static public readonly int MAX_MAJOR = 0xFFFF;
        static public readonly int MIN_MINOR = 0;
        static public readonly int MAX_MINOR = 0xFFFF;        

        // Member variables
        private BleDeviceData m_bleDeviceData = new BleDeviceData();
        private byte[] m_systemId  = new byte[Constants.SYSTEM_ID_LEN];
        private string m_fwRev = String.Empty;
        private string m_hwRev = String.Empty;
        private int m_advertIntervalMs = 0;
        private eTxPowerLevel m_txPowerDbm = 0;
        private int m_batteryThresholdPercent = -1;
        private int m_connMinIntervalMs = 0;
        private int m_connMaxIntervalMs = 0;
        private int m_connLatency = 0;
        private int m_connTimeoutMs = 0;
        private bool m_isProvisioned = false;
        private MeasuredTxPower_t m_measuredTxPowerSettings;
        private string m_minimumFwRev = String.Empty;
        private Peripherals m_peripherals = new Peripherals();
        private AdvSmStageSettings m_advSmStageSettings = new AdvSmStageSettings();

        // Constructors
        public ConfigurationData()
        {
            m_bleDeviceData = new BleDeviceData();
            IsUpdateRequired = false;
        }

        public ConfigurationData(ConfigurationData data)
        {
            Address = data.Address;
            SystemId = data.SystemId;
            FirmwareRevision = data.FirmwareRevision;
            HardwareRevision = data.HardwareRevision;
            DeviceName = data.DeviceName;
            AdvertisingIntervalMs = data.AdvertisingIntervalMs;
            TxPowerDbm = data.TxPowerDbm;
            BatteryThresholdPercent = data.BatteryThresholdPercent;
            ConnMinIntervalMs = data.ConnMinIntervalMs;
            ConnMaxIntervalMs = data.ConnMaxIntervalMs;
            ConnLatency = data.ConnLatency;
            ConnTimeoutMs = data.ConnTimeoutMs;
            IsProvisioned = data.IsProvisioned;
            UUID = data.UUID;
            Major = data.Major;
            Minor = data.Minor;
            IsUpdateRequired = false;
            IsBonded = data.IsBonded;
            MeasuredTxPowerMinus24dBm = data.MeasuredTxPowerMinus24dBm;
            MeasuredTxPowerMinus6dBm = data.MeasuredTxPowerMinus6dBm;
            MeasuredTxPower0dBm = data.MeasuredTxPower0dBm;
            MeasuredTxPower4dBm = data.MeasuredTxPower4dBm;
            MinimumFirmwareRevision = data.MinimumFirmwareRevision;
            PeripheralsSupported = data.PeripheralsSupported;
            AdvertisingStageSettings = data.AdvertisingStageSettings.DeepClone();
        }

        // Properties (autoimplemented)
        public bool IsBonded { get; set; }
        public bool IsUpdateRequired { get; set; }

        // Properties (manual)
        public DeviceAddress Address
        {
            get
            {
                return m_bleDeviceData.Address;
            }

            set
            {
                m_bleDeviceData.Address = value;
            }
        }

        public byte[] SystemId
        {
            get
            {
                return m_systemId;
            }

            set
            {
                m_systemId = value;
            }
        }
      
        public string FirmwareRevision
        {
            get
            {
                return m_fwRev;
            }

            set
            {
                m_fwRev = value;
            }
        }

        public string HardwareRevision
        {
            get
            {
                return m_hwRev;
            }

            set
            {
                if (m_hwRev.CompareTo(value) != 0)
                {
                    m_hwRev = value;
                    IsUpdateRequired = true;
                }
            }
        }

        public string DeviceName
        {
            get
            {
                return m_bleDeviceData.DeviceName;
            }

            set
            {
                if (m_bleDeviceData.DeviceName.CompareTo(value) != 0)
                {
                    m_bleDeviceData.DeviceName = value;
                    IsUpdateRequired = true;
                }
            }
        }

        public int AdvertisingIntervalMs
        {
            get
            {
                return m_advertIntervalMs;
            }

            set
            {
                if (m_advertIntervalMs != value)
                {
                    m_advertIntervalMs = value;
                    IsUpdateRequired = true;
                }
            }
        }

        public eTxPowerLevel TxPowerDbm
        {
            get
            {
                return m_txPowerDbm;
            }

            set
            {
                if (m_txPowerDbm != value)
                {
                    m_txPowerDbm = value;
                    IsUpdateRequired = true;
                }
            }
        }

        public int BatteryThresholdPercent
        {
            get
            {
                return m_batteryThresholdPercent;
            }

            set
            {
                if (m_batteryThresholdPercent != value)
                {
                    m_batteryThresholdPercent = value;
                    IsUpdateRequired = true;
                }
            }
        }

        public int ConnMinIntervalMs
        {
            get
            {
                return m_connMinIntervalMs;
            }

            set
            {
                if (m_connMinIntervalMs != value)
                {
                    m_connMinIntervalMs = value;
                    IsUpdateRequired = true;
                }
            }
        }

        public int ConnMaxIntervalMs
        {
            get
            {
                return m_connMaxIntervalMs;
            }

            set
            {
                if (m_connMaxIntervalMs != value)
                {
                    m_connMaxIntervalMs = value;
                    IsUpdateRequired = true;
                }
            }
        }

        public int ConnLatency
        {
            get
            {
                return m_connLatency;
            }

            set
            {
                if (m_connLatency != value)
                {
                    m_connLatency = value;
                    IsUpdateRequired = true;
                }
            }
        }

        public int ConnTimeoutMs
        {
            get
            {
                return m_connTimeoutMs;
            }

            set
            {
                if (m_connTimeoutMs != value)
                {
                    m_connTimeoutMs = value;
                    IsUpdateRequired = true;
                }
            }
        }

        public bool IsProvisioned
        {
            get
            {
                return m_isProvisioned;
            }

            set
            {
                if (m_isProvisioned != value)
                {
                    m_isProvisioned = value;
                    IsUpdateRequired = true;
                }
            }
        }

        public byte MeasuredTxPowerMinus24dBm
        {
            get
            {
                return m_measuredTxPowerSettings._Minus24dBm;
            }

            set
            {
                if (m_measuredTxPowerSettings._Minus24dBm != value)
                {
                    m_measuredTxPowerSettings._Minus24dBm = value;
                    IsUpdateRequired = true;
                }
            }
        }

        public byte MeasuredTxPowerMinus6dBm
        {
            get
            {
                return m_measuredTxPowerSettings._Minus6dBm;
            }

            set
            {
                if (m_measuredTxPowerSettings._Minus6dBm != value)
                {
                    m_measuredTxPowerSettings._Minus6dBm = value;
                    IsUpdateRequired = true;
                }
            }
        }

        public byte MeasuredTxPower0dBm
        {
            get
            {
                return m_measuredTxPowerSettings._0dBm;
            }

            set
            {
                if (m_measuredTxPowerSettings._0dBm != value)
                {
                    m_measuredTxPowerSettings._0dBm = value;
                    IsUpdateRequired = true;
                }
            }
        }

        public byte MeasuredTxPower4dBm
        {
            get
            {
                return m_measuredTxPowerSettings._4dBm;
            }

            set
            {
                if (m_measuredTxPowerSettings._4dBm != value)
                {
                    m_measuredTxPowerSettings._4dBm = value;
                    IsUpdateRequired = true;
                }
            }
        }

        public BeaconData _BeaconData
        {
            get
            {
                return m_bleDeviceData.BeaconData;
            }

            set
            {
                m_bleDeviceData.BeaconData = value;
            }
        }

        public DeviceUUID UUID
        {
            get
            {
                return m_bleDeviceData.BeaconData.UUID;
            }

            set
            {
                if (m_bleDeviceData.BeaconData.UUID != value)
                {
                    m_bleDeviceData.BeaconData.UUID = value;
                    IsUpdateRequired = true;
                }
            }
        }

        public UInt16 Major
        {
            get
            {
                return m_bleDeviceData.BeaconData.Major;
            }

            set
            {
                if (m_bleDeviceData.BeaconData.Major != value)
                {
                    m_bleDeviceData.BeaconData.Major = value;
                    IsUpdateRequired = true;
                }
            }
        }

        public UInt16 Minor
        {
            get
            {
                return m_bleDeviceData.BeaconData.Minor;
            }

            set
            {
                if (m_bleDeviceData.BeaconData.Minor != value)
                {
                    m_bleDeviceData.BeaconData.Minor = value;
                    IsUpdateRequired = true;
                }
            }
        }

        public string MinimumFirmwareRevision
        {
            get
            {
                return m_minimumFwRev;
            }

            set
            {
                m_minimumFwRev = value;
            }
        }

        public Peripherals PeripheralsSupported
        {
            get
            {
                return m_peripherals;
            }

            set
            {
                m_peripherals = value;
            }
        }

        public AdvSmStageSettings AdvertisingStageSettings
        {
            get
            {
                return m_advSmStageSettings;
            }

            set
            {
                m_advSmStageSettings = value;
            }
        }

        public eDeviceType GetDeviceType()
        {
            // Split string to extract components
            String[] revComponents = HardwareRevision.Split('_');

            // Ensure valid length
            if (revComponents.Length < 2)
                return eDeviceType.eInvalid;

            // Compare string to find device type
            if (revComponents[1] == "CC2540")
            {
                return eDeviceType.eCC2540;
            }
            else if (revComponents[1] == "CC2541")
            {
                return eDeviceType.eCC2541;
            }
            else
            {
                return eDeviceType.eInvalid;
            }
        }

        static public eFormFactor GetFormFactor(String s)
        {
            // Split string to extract components
            String[] revComponents = s.Split('_');

            // Ensure valid length
            if (revComponents.Length < 2)
                return eFormFactor.eInvalid;

            // Compare string to find device type
            if (revComponents[0] == "VTAG")
            {
                return eFormFactor.eVTAG;
            }
            else if (revComponents[0] == "F1")
            {
                return eFormFactor.eF1;
            }
            else if (revComponents[0] == "MiniBeacon")
            {
                return eFormFactor.eMiniBeacon;
            }
            else if (revComponents[0] == "WellCore")
            {
                return eFormFactor.eWellCore;
            }
            else
            {
                return eFormFactor.eInvalid;
            }
        }

        public eFormFactor GetFormFactor()
        {
            return GetFormFactor(HardwareRevision);
        }

        static public Revision_t GetFirmwareRevision(String s)
        {
            Revision_t rev = new Revision_t();

            // Split string to extract components
            String[] revComponents = s.Split('_');

            // Ensure valid length
            if (revComponents.Length < 2)
                return rev;

            String[] revNumbers = revComponents[1].Split(new string[] { "V", "." }, StringSplitOptions.RemoveEmptyEntries);
            if (revNumbers.Length < 2)
                return rev;

            rev.Major = Convert.ToInt32(revNumbers[0]);
            rev.Minor = Convert.ToInt32(revNumbers[1]);
            return rev;
        }

        public Revision_t GetFirmwareRevision()
        {
            return GetFirmwareRevision(FirmwareRevision);
        }

        static public bool isOadFirmware(String s)
        {
            // Split string to extract components
            String[] revComponents = s.Split('_');

            // Ensure valid length
            if (revComponents.Length < 3)
                return false;

            return revComponents[2].ToString().Equals("OAD");
        }

        public bool isOadFirmware()
        {
            return isOadFirmware(FirmwareRevision);
        }

        static public eUseCase GetUseCase(String s)
        {
            // Split string to extract components
            String[] revComponents = s.Split('_');

            // Ensure valid length
            if (revComponents.Length < 2)
                return eUseCase.eInvalid;

            // Compare string to find device type
            if (revComponents[0] == "Beacon")
            {
                return eUseCase.eBeacon;
            }
            if (revComponents[0] == "Auth-Beacon")
            {
                return eUseCase.eAuthBeacon;
            }
            else if (revComponents[0] == "Blu")
            {
                return eUseCase.eBlu;
            }
            else if (revComponents[0] == "Boot")
            {
                return eUseCase.eBoot;
            }
            else
            {
                return eUseCase.eInvalid;
            }
        }

        public eUseCase GetUseCase()
        {
            return GetUseCase(FirmwareRevision);
        }

        public String GetTxPowerLevelString()
        {
            if (TxPowerDbm == eTxPowerLevel.eInvalid)
            {
                return "Invalid";
            }
            else
            {
                try
                {
                    return Enum.Format(typeof(ConfigurationData.eTxPowerLevel), TxPowerDbm, "d");
                }
                catch
                {
                    return "Invalid";
                }
            }
        }

        public String GetBatteryThresholdPercentString()
        {
            if ((BatteryThresholdPercent != -1) && (BatteryThresholdPercent != 0))
            {
                return BatteryThresholdPercent.ToString();
            }
            else
            {
                return "N/A";
            }
        }

        public String GetMeasuredTxPowerMinus24dBmString()
        {
            if (MeasuredTxPowerMinus24dBm != 0)
                return Helper.ConvertFromTwoComplement(MeasuredTxPowerMinus24dBm).ToString();
            else
                return "N/A";
        }

        public String GetMeasuredTxPowerMinus6dBmString()
        {
            if (MeasuredTxPowerMinus6dBm != 0)
                return Helper.ConvertFromTwoComplement(MeasuredTxPowerMinus6dBm).ToString();
            else
                return "N/A";
        }

        public string GetMeasuredTxPower0dBmString()
        {
            if (MeasuredTxPower0dBm != 0)
                return Helper.ConvertFromTwoComplement(MeasuredTxPower0dBm).ToString();
            else
                return "N/A";
        }

        public string GetMeasuredTxPower4dBmString()
        {
            if (MeasuredTxPower4dBm != 0)
                return Helper.ConvertFromTwoComplement(MeasuredTxPower4dBm).ToString();
            else
                return "N/A";
        }

    }
}

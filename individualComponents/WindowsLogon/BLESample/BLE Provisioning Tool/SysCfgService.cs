using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using BLE_API;

namespace BLE_Provisioning_Tool
{
    class SysCfgService
    {
        static public readonly UInt16 SERV_UUID = 0xA500; // System Configuration
        static private readonly UInt16 ATT_BUFF_UUID = 0xA501; // Buffer Attribute
        static private readonly UInt16 ATT_ENCRBUFF_UUID = 0xA502; // Encrypted Buffer Attribute
        static private readonly UInt16 ATT_MIN_FW_REV_UUID = 0xA503; // Minimum Firmware Revision Attribute
        static private readonly UInt16 ATT_PERIPHERALS_UUID = 0xA504; // Peripherals Attribute

        public enum CmdId
        {
            Invalidate = 0x01,
            ResetProv = 0x02,
            InvalidateEx = 0x03,            
            GetMajor = 0xa1,
            GetMinor,
            GetTxPower,
            GetAdvInt,
            GetBattThres,
            GetDeviceUuid,
            GetDeviceName,
            GetConnParams,
            GetIsProvisioned,
            GetMeasuredTxPowerSettings,
            GetAdvSmSettings,
        };

        public enum EncrCmdId
        {
            SetMajor = 0x01,
            SetMinor,
            SetTxPower,
            SetAdvInt,
            SetBattThres,
            SetDeviceUuid,
            SetDeviceName,
            SetConnParams,
            FinializeProv,
            SetMeasuredTxPowerSettings,
            SetAdvSmSettings,
        };

        public enum eRawTxPower
        {
            eMinus23dBm = 0,
            eMinus6dBm = 1,
            e0dBm = 2,
            e4dBm = 3,
            eInvalid
        };

        static public async Task<bool> ReadAllAtt(BLE.V2_2.Device connectedDevice, ConfigurationData cfgData)
        {
            if (!await ReadAttRequest(connectedDevice, CmdId.GetMajor, cfgData)) return false; // Not required if found in advertising data
            if (!await ReadAttRequest(connectedDevice, CmdId.GetMinor, cfgData)) return false; // Not required if found in advertising data            
            if (!await ReadAttRequest(connectedDevice, CmdId.GetAdvInt, cfgData)) return false;
            if (!await ReadAttRequest(connectedDevice, CmdId.GetTxPower, cfgData)) return false;
            if (!await ReadAttRequest(connectedDevice, CmdId.GetBattThres, cfgData)) return false;
            if (!await ReadAttRequest(connectedDevice, CmdId.GetDeviceUuid, cfgData)) return false; // Not required if found in advertising data
            if (!await ReadAttRequest(connectedDevice, CmdId.GetDeviceName, cfgData)) return false; // Not required if found in advertising data
            if (!await ReadAttRequest(connectedDevice, CmdId.GetConnParams, cfgData)) return false;

            // Attempt to retrieve Provisioning Flag
            if (!await ReadAttRequest(connectedDevice, CmdId.GetIsProvisioned, cfgData))
            {
                cfgData.IsProvisioned = false;
            }

            // Attempt to retrieve Measured Tx Power Settings
            if (!await ReadAttRequest(connectedDevice, CmdId.GetMeasuredTxPowerSettings, cfgData))
            {
                cfgData.MeasuredTxPowerMinus24dBm = 0;
                cfgData.MeasuredTxPowerMinus6dBm = 0;
                cfgData.MeasuredTxPower0dBm = 0;
                cfgData.MeasuredTxPower4dBm = 0;
            }

            // Attempt to retrieve Minimum Firmware Revision
            {
                var result = await ReadMinimumFirmwareRevision(connectedDevice);
                cfgData.MinimumFirmwareRevision = result.Item2;
            }

            // Attempt to retrieve peripherals supported on the device
            {
                var result = await ReadPeripherals(connectedDevice);
                cfgData.PeripheralsSupported = result.Item2;
            }

            // Attempt to retrieve Advertising State Machine Settings
            await ReadAttRequest(connectedDevice, CmdId.GetAdvSmSettings, cfgData);

            return true;
        }

        static public async Task<Tuple<bool, string>> ReadMinimumFirmwareRevision(BLE.V2_2.Device connectedDevice)
        {
            var result = await BLE_AdvFunctions.ReadRequest(connectedDevice, SysCfgService.ATT_MIN_FW_REV_UUID, BLE_AdvFunctions.UNDEFINED_LENGTH);
            if (result.Item1)
            {
                return new Tuple<bool,string>(result.Item1, System.Text.Encoding.UTF8.GetString(result.Item2));
            }
            else
            {
                return new Tuple<bool, string>(result.Item1, "");
            }
        }

        static public async Task<bool> ReadAttRequest(BLE.V2_2.Device connectedDevice, CmdId cmd, ConfigurationData cfgData)
        {
            byte[] cmdByte = new byte[] { (byte)cmd };
            int expectedDataByteLength = GetBytesLength(cmd);            
            var result = await BLE_AdvFunctions.MT_ReadRequest(connectedDevice, SysCfgService.ATT_BUFF_UUID, cmdByte, expectedDataByteLength);
            if (!result.Item1)
            {
                return false;
            }

            // Ensure result length is correct
            if (result.Item2.Length < expectedDataByteLength)
            {
                return false;
            }

            try {
                switch (cmd)
                {
                    case CmdId.GetMajor:
                    {
                        // Convert to little endian
                        byte[] reverseData = result.Item2.Take(sizeof(UInt16)).ToArray();
                        Array.Reverse(reverseData);
                        cfgData.Major = BitConverter.ToUInt16(reverseData, 0);
                        break;
                    }
                    case CmdId.GetMinor:
                    {
                        // Convert to little endian
                        byte[] reverseData = result.Item2.Take(sizeof(UInt16)).ToArray();
                        Array.Reverse(reverseData);
                        cfgData.Minor = BitConverter.ToUInt16(reverseData, 0);
                        break;
                    }
                    case CmdId.GetTxPower:
                        cfgData.TxPowerDbm = ConvertTxPowerFromRaw(result.Item2[0]);
                        break;
                    
                    case CmdId.GetAdvInt:
                        cfgData.AdvertisingIntervalMs = ConvertAdvertIntervalToMs(BitConverter.ToUInt16(result.Item2.Take(sizeof(UInt16)).ToArray(), 0));
                        break;

                    case CmdId.GetBattThres:
                        cfgData.BatteryThresholdPercent = result.Item2[0];
                        break;

                    case CmdId.GetDeviceUuid:
                        cfgData.UUID.Value = result.Item2.Take(Constants.DEVICE_UUID_LEN).ToArray();
                        break;
                        
                    case CmdId.GetDeviceName:
                        cfgData.DeviceName = System.Text.Encoding.UTF8.GetString(result.Item2);
                        break;

                    case CmdId.GetConnParams:
                    {
                        Int16 offset = 0;
                        cfgData.ConnMinIntervalMs = ConvertConnIntervalToMs(BitConverter.ToUInt16(result.Item2.Skip(offset).Take(sizeof(UInt16)).ToArray(), 0));
                        cfgData.ConnMaxIntervalMs = ConvertConnIntervalToMs(BitConverter.ToUInt16(result.Item2.Skip(offset += sizeof(UInt16)).Take(sizeof(UInt16)).ToArray(), 0));
                        cfgData.ConnLatency = BitConverter.ToUInt16(result.Item2.Skip(offset += sizeof(UInt16)).Take(sizeof(UInt16)).ToArray(), 0);
                        cfgData.ConnTimeoutMs = ConvertConnTimeoutToMs(BitConverter.ToUInt16(result.Item2.Skip(offset += sizeof(UInt16)).Take(sizeof(UInt16)).ToArray(), 0));;
                        break;
                    }
                    case CmdId.GetIsProvisioned:
                        cfgData.IsProvisioned = Convert.ToBoolean(result.Item2[0]);
                        break;

                    case CmdId.GetMeasuredTxPowerSettings:
                        cfgData.MeasuredTxPowerMinus24dBm = result.Item2[0];
                        cfgData.MeasuredTxPowerMinus6dBm = result.Item2[1];
                        cfgData.MeasuredTxPower0dBm = result.Item2[2];
                        cfgData.MeasuredTxPower4dBm = result.Item2[3];
                        break;

                    case CmdId.GetAdvSmSettings:
                        cfgData.AdvertisingStageSettings.Deserialize(result.Item2);
                        break;

                    default:
                        throw new System.NotSupportedException();
                }
                return true;
            }
            catch
            {
                return false;
            }
        }

        static public async Task<bool> UpdateConfiguration(BLE.V2_2.Device connectedDevice, ConfigurationData oldCfgData, ConfigurationData newCfgData, SessionData sessionData)
        {            
            // Update affected members
            if (oldCfgData.Major != newCfgData.Major)
            {
                // Convert back to big endian
                byte[] reverseData = BitConverter.GetBytes(Convert.ToUInt16(newCfgData.Major));
                Array.Reverse(reverseData);

                // Ready plaintext
                byte[] plaintext = Helper.BuildByte( new byte[][] {
                        new byte[] { (byte)EncrCmdId.SetMajor },
                        reverseData
                });

                // Create encrypted buffer
                byte[] encryptedBuff;
                sessionData.CreateEncryptedBuffer(SessionData.EnrcryptionKeyId.SESSION_ENCR_CMD_KEY_ID, ref plaintext, out encryptedBuff);

                // Write to encrypted buffer
                if (!await BLE_AdvFunctions.WriteRequest(connectedDevice, SysCfgService.ATT_ENCRBUFF_UUID, encryptedBuff)) return false;
            }

            if (oldCfgData.Minor != newCfgData.Minor)
            {
                // Convert back to big endian
                byte[] reverseData = BitConverter.GetBytes(Convert.ToUInt16(newCfgData.Minor));
                Array.Reverse(reverseData);

                // Ready plaintext
                byte[] plaintext = Helper.BuildByte(new byte[][] {
                        new byte[] { (byte)EncrCmdId.SetMinor },
                        reverseData
                });

                // Create encrypted buffer
                byte[] encryptedBuff;
                sessionData.CreateEncryptedBuffer(SessionData.EnrcryptionKeyId.SESSION_ENCR_CMD_KEY_ID, ref plaintext, out encryptedBuff);

                // Write to encrypted buffer
                if (!await BLE_AdvFunctions.WriteRequest(connectedDevice, SysCfgService.ATT_ENCRBUFF_UUID, encryptedBuff)) return false;
            }

            if (oldCfgData.TxPowerDbm != newCfgData.TxPowerDbm)
            {
                // Ready plaintext
                byte[] plaintext = Helper.BuildByte(new byte[][] {
                        new byte[] { (byte)EncrCmdId.SetTxPower },
                        new byte[] { (byte)ConvertTxPowerToRaw(newCfgData.TxPowerDbm) }
                });

                // Create encrypted buffer
                byte[] encryptedBuff;
                sessionData.CreateEncryptedBuffer(SessionData.EnrcryptionKeyId.SESSION_ENCR_CMD_KEY_ID, ref plaintext, out encryptedBuff);

                // Write to encrypted buffer
                if (!await BLE_AdvFunctions.WriteRequest(connectedDevice, SysCfgService.ATT_ENCRBUFF_UUID, encryptedBuff)) return false;
            }

            if (oldCfgData.BatteryThresholdPercent != newCfgData.BatteryThresholdPercent)
            {
                // Ready plaintext
                byte[] plaintext = Helper.BuildByte(new byte[][] {
                        new byte[] { (byte)EncrCmdId.SetBattThres },
                        BitConverter.GetBytes(Convert.ToUInt16(newCfgData.BatteryThresholdPercent))
                });

                // Create encrypted buffer
                byte[] encryptedBuff;
                sessionData.CreateEncryptedBuffer(SessionData.EnrcryptionKeyId.SESSION_ENCR_CMD_KEY_ID, ref plaintext, out encryptedBuff);

                // Write to encrypted buffer
                if (!await BLE_AdvFunctions.WriteRequest(connectedDevice, SysCfgService.ATT_ENCRBUFF_UUID, encryptedBuff)) return false;
            }

            if (oldCfgData.UUID != newCfgData.UUID)
            {
                // Ready plaintext
                byte[] plaintext = Helper.BuildByte(new byte[][] {
                        new byte[] { (byte)EncrCmdId.SetDeviceUuid },
                        newCfgData.UUID.Value
                });

                // Create encrypted buffer
                byte[] encryptedBuff;
                sessionData.CreateEncryptedBuffer(SessionData.EnrcryptionKeyId.SESSION_ENCR_CMD_KEY_ID, ref plaintext, out encryptedBuff);

                // Write to encrypted buffer
                if (!await BLE_AdvFunctions.WriteRequest(connectedDevice, SysCfgService.ATT_ENCRBUFF_UUID, encryptedBuff)) return false;
            }

#if true // Prevent from being changed
            if (oldCfgData.DeviceName != newCfgData.DeviceName)
            {
                // Ready plaintext
                byte[] plaintext = Helper.BuildByte(new byte[][] {
                        new byte[] { (byte)EncrCmdId.SetDeviceName },
                        Encoding.UTF8.GetBytes(newCfgData.DeviceName + '\0') // Ensure there is a null terminator
                });

                // Create encrypted buffer
                byte[] encryptedBuff;
                sessionData.CreateEncryptedBuffer(SessionData.EnrcryptionKeyId.SESSION_ENCR_CMD_KEY_ID, ref plaintext, out encryptedBuff);

                // Write to encrypted buffer
                if (!await BLE_AdvFunctions.WriteRequest(connectedDevice, SysCfgService.ATT_ENCRBUFF_UUID, encryptedBuff)) return false;
            }
#endif

            if ((oldCfgData.ConnMinIntervalMs != newCfgData.ConnMinIntervalMs) ||
                    (oldCfgData.ConnMaxIntervalMs != newCfgData.ConnMaxIntervalMs) ||
                    (oldCfgData.ConnLatency != newCfgData.ConnLatency) ||
                    (oldCfgData.ConnTimeoutMs != newCfgData.ConnTimeoutMs))
            {
                // Ready plaintext
                byte[] plaintext = Helper.BuildByte(new byte[][] {
                        new byte[] { (byte)EncrCmdId.SetConnParams },
                        ConvertConnIntervalToRaw(newCfgData.ConnMinIntervalMs),
                        ConvertConnIntervalToRaw(newCfgData.ConnMaxIntervalMs),
                        BitConverter.GetBytes(Convert.ToUInt16(newCfgData.ConnLatency)),
                        ConvertConnTimeoutToRaw(newCfgData.ConnTimeoutMs)
                });

                // Create encrypted buffer
                byte[] encryptedBuff;
                sessionData.CreateEncryptedBuffer(SessionData.EnrcryptionKeyId.SESSION_ENCR_CMD_KEY_ID, ref plaintext, out encryptedBuff);

                // Write to encrypted buffer
                if (!await BLE_AdvFunctions.WriteRequest(connectedDevice, SysCfgService.ATT_ENCRBUFF_UUID, encryptedBuff)) return false;
            }

            if ((oldCfgData.MeasuredTxPowerMinus24dBm != newCfgData.MeasuredTxPowerMinus24dBm) ||
                    (oldCfgData.MeasuredTxPowerMinus6dBm != newCfgData.MeasuredTxPowerMinus6dBm) ||
                    (oldCfgData.MeasuredTxPower0dBm != newCfgData.MeasuredTxPower0dBm) ||
                    (oldCfgData.MeasuredTxPower4dBm != newCfgData.MeasuredTxPower4dBm))
            {
                // Ready plaintext
                byte[] plaintext = Helper.BuildByte(new byte[][] {
                        new byte[] { (byte)EncrCmdId.SetMeasuredTxPowerSettings },
                        new byte[] { (byte)newCfgData.MeasuredTxPowerMinus24dBm },
                        new byte[] { (byte)newCfgData.MeasuredTxPowerMinus6dBm },
                        new byte[] { (byte)newCfgData.MeasuredTxPower0dBm },
                        new byte[] { (byte)newCfgData.MeasuredTxPower4dBm },
                });

                // Create encrypted buffer
                byte[] encryptedBuff;
                sessionData.CreateEncryptedBuffer(SessionData.EnrcryptionKeyId.SESSION_ENCR_CMD_KEY_ID, ref plaintext, out encryptedBuff);

                // Write to encrypted buffer
                if (!await BLE_AdvFunctions.WriteRequest(connectedDevice, SysCfgService.ATT_ENCRBUFF_UUID, encryptedBuff)) return false;
            }

            // Use Advertising Stage Settings for V1.11+
            var fmRev = oldCfgData.GetFirmwareRevision();
            if (fmRev.Major <= 1 && fmRev.Minor <= 11)
            {
                if (oldCfgData.AdvertisingIntervalMs != newCfgData.AdvertisingIntervalMs)
                {
                    // Ready plaintext
                    byte[] plaintext = Helper.BuildByte(new byte[][] {
                            new byte[] { (byte)EncrCmdId.SetAdvInt },
                            BitConverter.GetBytes(ConvertAdvertIntervalToRaw(newCfgData.AdvertisingIntervalMs))
                    });

                    // Create encrypted buffer
                    byte[] encryptedBuff;
                    sessionData.CreateEncryptedBuffer(SessionData.EnrcryptionKeyId.SESSION_ENCR_CMD_KEY_ID, ref plaintext, out encryptedBuff);

                    // Write to encrypted buffer
                    if (!await BLE_AdvFunctions.WriteRequest(connectedDevice, SysCfgService.ATT_ENCRBUFF_UUID, encryptedBuff)) return false;
                }
            }
            else
            {
                // Make necessary changes for beacon use case to pass data validation
                if ((oldCfgData.GetUseCase() == ConfigurationData.eUseCase.eAuthBeacon) || (oldCfgData.GetUseCase() == ConfigurationData.eUseCase.eBeacon))
                {
                    newCfgData.AdvertisingStageSettings.SetIdleTimeoutMs(ConfigurationData.AdvSmStageSettings.Stages.Stage0, Convert.ToUInt16(ConfigurationData.AdvSmStageSettings.MIN_ADVSM_IDLETIMEOUT_MS));
                    newCfgData.AdvertisingStageSettings.SetIdleTimeoutMs(ConfigurationData.AdvSmStageSettings.Stages.Stage1, SysCfgService.ConvertAdvertIntervalToRaw((newCfgData.AdvertisingStageSettings.Get(ConfigurationData.AdvSmStageSettings.Stages.Stage1)._intervalRaw))); // Must be same as interval to pass validation
                    // Skip setting interval as already done
                    newCfgData.AdvertisingStageSettings.SetIdleTimeoutMs(ConfigurationData.AdvSmStageSettings.Stages.Stage2, Convert.ToUInt16(ConfigurationData.AdvSmStageSettings.MIN_ADVSM_IDLETIMEOUT_MS));
                    newCfgData.AdvertisingStageSettings.SetIntervalMs(ConfigurationData.AdvSmStageSettings.Stages.Stage2, Convert.ToUInt16(ConfigurationData.AdvSmStageSettings.MIN_ADVSM_INTERVAL_MS));
                    newCfgData.AdvertisingStageSettings.SetIdleTimeoutMs(ConfigurationData.AdvSmStageSettings.Stages.Stage3, Convert.ToUInt16(ConfigurationData.AdvSmStageSettings.MIN_ADVSM_IDLETIMEOUT_MS));
                    newCfgData.AdvertisingStageSettings.SetIntervalMs(ConfigurationData.AdvSmStageSettings.Stages.Stage3, Convert.ToUInt16(ConfigurationData.AdvSmStageSettings.MIN_ADVSM_INTERVAL_MS));
                }

                // Check if update is necessary
                if (!oldCfgData.AdvertisingStageSettings.IsEqual(newCfgData.AdvertisingStageSettings))
                {
                    byte[] plaintext = Helper.BuildByte(new byte[][] {
                            new byte[] { (byte)EncrCmdId.SetAdvSmSettings },
                            newCfgData.AdvertisingStageSettings.Serialize(),
                    });

                    // Create encrypted buffer
                    byte[] encryptedBuff;
                    sessionData.CreateEncryptedBuffer(SessionData.EnrcryptionKeyId.SESSION_ENCR_CMD_KEY_ID, ref plaintext, out encryptedBuff);

                    // Write to encrypted buffer
                    if (!await BLE_AdvFunctions.WriteRequest(connectedDevice, SysCfgService.ATT_ENCRBUFF_UUID, encryptedBuff)) return false;
                }
            }

            return true;
        }

        static public async Task<bool> InvalidateDevice(BLE.V2_2.Device connectedDevice)
        {
            return await BLE_AdvFunctions.WriteRequest(connectedDevice, SysCfgService.ATT_BUFF_UUID, new byte[] { (byte)CmdId.Invalidate });
        }

        static public async Task<bool> InvalidateDeviceEx(BLE.V2_2.Device connectedDevice)
        {
            return await BLE_AdvFunctions.WriteRequest(connectedDevice, SysCfgService.ATT_BUFF_UUID, new byte[] { (byte)CmdId.InvalidateEx });
        }

        static public async Task<bool> ResetProvision(BLE.V2_2.Device connectedDevice)
        {
            return await BLE_AdvFunctions.WriteRequest(connectedDevice, SysCfgService.ATT_BUFF_UUID, new byte[] { (byte)CmdId.ResetProv });
        }

        static public async Task<bool> FinializeProvision(BLE.V2_2.Device connectedDevice, SessionData sessionData)
        {
            // Ready plaintext
            byte[] plaintext = Helper.BuildByte(new byte[][] {
                        new byte[] { (byte)EncrCmdId.FinializeProv },
                        new byte[] { 0 } // Dummy
                });

            // Create encrypted buffer
            byte[] encryptedBuff;
            sessionData.CreateEncryptedBuffer(SessionData.EnrcryptionKeyId.SESSION_ENCR_CMD_KEY_ID, ref plaintext, out encryptedBuff);

            // Write to encrypted buffer
            return await BLE_AdvFunctions.WriteRequest(connectedDevice, SysCfgService.ATT_ENCRBUFF_UUID, encryptedBuff);
        }

        static public async Task<Tuple<bool, ConfigurationData.Peripherals>> ReadPeripherals(BLE.V2_2.Device connectedDevice)
        {
            var result = await BLE_AdvFunctions.ReadRequest(connectedDevice, SysCfgService.ATT_PERIPHERALS_UUID, sizeof(UInt16));
            if (result.Item1)
            {
                byte[] reverseData = result.Item2.Take(sizeof(UInt16)).ToArray();
                Array.Reverse(reverseData); // Convert to little endian
                return new Tuple<bool, ConfigurationData.Peripherals>(true, new ConfigurationData.Peripherals(BitConverter.ToUInt16(reverseData, 0)));
            }
            return new Tuple<bool, ConfigurationData.Peripherals>(false, null);
        }

        static private int GetBytesLength(CmdId cmd)
        {
            switch (cmd)
            {
                case CmdId.GetMajor:
                case CmdId.GetMinor:
                case CmdId.GetAdvInt:
                    return sizeof(UInt16);

                case CmdId.GetTxPower:
                case CmdId.GetBattThres:
                    return sizeof(byte);

                case CmdId.GetDeviceUuid:
                    return Constants.DEVICE_UUID_LEN;

                case CmdId.GetDeviceName:
                    return ConfigurationData.MAX_DEV_NAME_LEN;

                case CmdId.GetConnParams:
                    return sizeof(UInt16) * 4;

                case CmdId.GetIsProvisioned:
                    return sizeof(bool);

                case CmdId.GetMeasuredTxPowerSettings:
                    return sizeof(byte) * 4;

                case CmdId.GetAdvSmSettings:
                    return ConfigurationData.AdvSmStageSettings.Size();

                default:
                    return 0;
            }
        }

        static public int ConvertAdvertIntervalToMs(UInt16 raw)
        {
            return (int)Math.Round(raw * 0.625); // Each unit is 625us
        }

        static public UInt16 ConvertAdvertIntervalToRaw(int ms)
        {
            return Convert.ToUInt16(Math.Round(ms / 0.625)); // Each unit is 625us
        }

        static public int ConvertConnIntervalToMs(UInt16 raw)
        {
            return (int)Math.Round(raw * 1.25); // Each unit is 1.25ms
        }

        static public byte[] ConvertConnIntervalToRaw(int ms)
        {
            return BitConverter.GetBytes(Convert.ToUInt16(Math.Round(ms / 1.25))); // Each unit is 1.25ms
        }

        static public int ConvertConnTimeoutToMs(UInt16 raw)
        {
            return (int)Math.Round(raw * 10.0); // Each unit is 10ms
        }

        static public byte[] ConvertConnTimeoutToRaw(int ms)
        {
            return BitConverter.GetBytes(Convert.ToUInt16(Math.Round(ms / 10.0))); // Each unit is 10ms
        }

        static public eRawTxPower ConvertTxPowerToRaw(ConfigurationData.eTxPowerLevel txPowerLevel)
        {
            switch (txPowerLevel)
            {
                case ConfigurationData.eTxPowerLevel.eMinus23dBm:
                    return eRawTxPower.eMinus23dBm;
                case ConfigurationData.eTxPowerLevel.eMinus6dBm:
                    return eRawTxPower.eMinus6dBm;
                case ConfigurationData.eTxPowerLevel.e0dBm:
                    return eRawTxPower.e0dBm;
                case ConfigurationData.eTxPowerLevel.e4dBm:
                    return eRawTxPower.e4dBm;
                default:
                    return eRawTxPower.eInvalid;
            }
        }

        static public ConfigurationData.eTxPowerLevel ConvertTxPowerFromRaw(byte rawTxPower)
        {
            switch ((eRawTxPower)rawTxPower)
            {
                case eRawTxPower.eMinus23dBm:
                    return ConfigurationData.eTxPowerLevel.eMinus23dBm;
                case eRawTxPower.eMinus6dBm:
                    return ConfigurationData.eTxPowerLevel.eMinus6dBm;
                case eRawTxPower.e0dBm:
                    return ConfigurationData.eTxPowerLevel.e0dBm;
                case eRawTxPower.e4dBm:
                    return ConfigurationData.eTxPowerLevel.e4dBm;
                default:
                    return ConfigurationData.eTxPowerLevel.eInvalid;
            }
        }
    }
}

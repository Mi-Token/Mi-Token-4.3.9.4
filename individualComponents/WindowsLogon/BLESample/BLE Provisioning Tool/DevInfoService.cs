using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using BLE_API;

namespace BLE_Provisioning_Tool
{
    class DevInfoService
    {
        static public readonly UInt16 SERV_UUID = 0x180A; // Device Information
        static public readonly UInt16 ATT_SYSID_UUID = 0x2A23; // System ID
        static public readonly UInt16 ATT_FWREV_UUID = 0x2A26; // Firmware Revision String
        static public readonly UInt16 ATT_HWREV_UUID = 0x2A27; // Hardware Revision String

        static public async Task<bool> ReadAllAtt(BLE.V2_2.Device connectedDevice, ConfigurationData cfgData)
        {
            // Retrieve System ID
            {
                var result = await BLE_AdvFunctions.ReadRequest(connectedDevice, DevInfoService.ATT_SYSID_UUID, BLE_AdvFunctions.UNDEFINED_LENGTH);
                if (result.Item1)
                {
                    cfgData.SystemId = result.Item2.ToArray();
                }
                else
                {
                    return false;
                }
            }

            // Retrieve Firmware Revision
            {
                var result = await BLE_AdvFunctions.ReadRequest(connectedDevice, DevInfoService.ATT_FWREV_UUID, BLE_AdvFunctions.UNDEFINED_LENGTH);
                if (result.Item1)
                {
                    cfgData.FirmwareRevision = System.Text.Encoding.UTF8.GetString(result.Item2);
                }
                else
                {
                    return false;
                }
            }

            // Retrieve Hardware Revision
            {
                var result = await BLE_AdvFunctions.ReadRequest(connectedDevice, DevInfoService.ATT_HWREV_UUID, BLE_AdvFunctions.UNDEFINED_LENGTH);
                if (result.Item1)
                {
                    cfgData.HardwareRevision = System.Text.Encoding.UTF8.GetString(result.Item2);
                }
                else
                {
                    return false;
                }
            }

            return true;
        }

        static public async Task<ConfigurationData.eFormFactor> ReadHardwareRevision(BLE.V2_2.Device connectedDevice)
        {
            // Retrieve Hardware Revision
            var result = await BLE_AdvFunctions.ReadRequest(connectedDevice, DevInfoService.ATT_HWREV_UUID, BLE_AdvFunctions.UNDEFINED_LENGTH);
            if (result.Item1)
            {
                return ConfigurationData.GetFormFactor(System.Text.Encoding.UTF8.GetString(result.Item2));
            }
            return ConfigurationData.eFormFactor.eInvalid;
        }
    }
}

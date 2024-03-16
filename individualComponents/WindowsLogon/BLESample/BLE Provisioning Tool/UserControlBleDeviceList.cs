using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace BLE_Provisioning_Tool
{
    class UserControlBleDeviceList
    {
        private List<UserControlBleDevice> m_ucBleDevices = new List<UserControlBleDevice>();

        public bool Add(ref FlowLayoutPanel refPanel, BleDeviceData data, out UserControlBleDevice result)
        {
            // Attempt to find device
            UserControlBleDevice ucBleDevice = m_ucBleDevices.Find(ByAddres(data));
            if (ucBleDevice == null)
            {
                // New BLE Device
                UserControlBleDevice newUcBleDevice = new UserControlBleDevice(data);
                m_ucBleDevices.Add(newUcBleDevice);                
                refPanel.Controls.Add(newUcBleDevice);
                result = newUcBleDevice;
                return true;
            }
            else
            {
                // Ensure that data that is not received from from advertising data is updated accordingly
                data.IsSecureBondSvcExists = GetIsSecureBondSvcExists(data);
                data.IsDevInfoSvcExists = GetIsDevInfoSvcExists(data);
                data.IsSysCfgSvcExists = GetIsSysCfgSvcExists(data);
                data.IsAuthenSvcExists = GetIsAuthenSvcExists(data);
                data.IsOadSvcExists = GetIsOadSvcExists(data);
                data.IsBuzzerExists = GetIsBuzzerExists(data);
                data.IsProvisioned = GetIsProvisioned(data);
                data.IsOadFirmware = GetIsOadFirmware(data);
                data.UseCase = GetUseCase(data);
                data.IsAlreadyBonded = GetIsAlreadyBonded(data);

                // Update Exisiting BLE Device
                ucBleDevice.UpdateData(data);
                result = ucBleDevice;
                return false;
            }
        }

        public bool Remove(ref FlowLayoutPanel refPanel, BleDeviceData data)
        {
            UserControlBleDevice ucBleDevice = m_ucBleDevices.Find(ByAddres(data));
            if (ucBleDevice == null)
            {
                return false;
            }
            m_ucBleDevices.Remove(ucBleDevice);
            refPanel.Controls.Remove(ucBleDevice);
            return true;
        }

        public void Reset()
        {
            m_ucBleDevices.Clear();
        }

        public bool UpdateProvision(BleDeviceData bleData, bool isEnabled)
        {
            UserControlBleDevice ucBleDevice = m_ucBleDevices.Find(ByAddres(bleData));
            if (ucBleDevice == null)
            {
                return false;
            }

            ucBleDevice.IsProvisioned = true;
            ucBleDevice.RefreshServiceControls();
            return true;
        }

        private Boolean GetIsSecureBondSvcExists(BleDeviceData bleData)
        {
            UserControlBleDevice ucBleDevice = m_ucBleDevices.Find(ByAddres(bleData));
            if (ucBleDevice == null)
            {
                return false;
            }

            return ucBleDevice.IsSecureBondSvcExists;
        }

        private Boolean GetIsDevInfoSvcExists(BleDeviceData bleData)
        {
            UserControlBleDevice ucBleDevice = m_ucBleDevices.Find(ByAddres(bleData));
            if (ucBleDevice == null)
            {
                return false;
            }

            return ucBleDevice.IsDevInfoSvcExists;
        }

        private Boolean GetIsSysCfgSvcExists(BleDeviceData bleData)
        {
            UserControlBleDevice ucBleDevice = m_ucBleDevices.Find(ByAddres(bleData));
            if (ucBleDevice == null)
            {
                return false;
            }

            return ucBleDevice.IsSysCfgSvcExists;
        }

        private Boolean GetIsAuthenSvcExists(BleDeviceData bleData)
        {
            UserControlBleDevice ucBleDevice = m_ucBleDevices.Find(ByAddres(bleData));
            if (ucBleDevice == null)
            {
                return false;
            }

            return ucBleDevice.IsAuthenSvcExists;
        }

        private Boolean GetIsOadSvcExists(BleDeviceData bleData)
        {
            UserControlBleDevice ucBleDevice = m_ucBleDevices.Find(ByAddres(bleData));
            if (ucBleDevice == null)
            {
                return false;
            }

            return ucBleDevice.IsOadSvcExists;
        }

        private Boolean GetIsBuzzerExists(BleDeviceData bleData)
        {
            UserControlBleDevice ucBleDevice = m_ucBleDevices.Find(ByAddres(bleData));
            if (ucBleDevice == null)
            {
                return false;
            }

            return ucBleDevice.IsBuzzerExists;
        }

        private Boolean GetIsProvisioned(BleDeviceData bleData)
        {
            UserControlBleDevice ucBleDevice = m_ucBleDevices.Find(ByAddres(bleData));
            if (ucBleDevice == null)
            {
                return false;
            }

            return ucBleDevice.IsProvisioned;
        }

        private Boolean GetIsOadFirmware(BleDeviceData bleData)
        {
            UserControlBleDevice ucBleDevice = m_ucBleDevices.Find(ByAddres(bleData));
            if (ucBleDevice == null)
            {
                return false;
            }

            return ucBleDevice.IsOadFirmware;
        }

        private ConfigurationData.eUseCase GetUseCase(BleDeviceData bleData)
        {
            UserControlBleDevice ucBleDevice = m_ucBleDevices.Find(ByAddres(bleData));
            if (ucBleDevice == null)
            {
                return ConfigurationData.eUseCase.eInvalid;
            }

            return ucBleDevice.UseCase;
        }

        private Boolean GetIsAlreadyBonded(BleDeviceData bleData)
        {
            UserControlBleDevice ucBleDevice = m_ucBleDevices.Find(ByAddres(bleData));
            if (ucBleDevice == null)
            {
                return false;
            }

            return ucBleDevice.IsAlreadyBonded;
        }

        static private Predicate<UserControlBleDevice> ByAddressAndName(BleDeviceData data)
        {
            return delegate(UserControlBleDevice ucBleDevice)
            {
                return (ucBleDevice.Address.Value == data.Address.Value) && (ucBleDevice.DeviceName.CompareTo(data.DeviceName) == 0);
            };
        }

        static private Predicate<UserControlBleDevice> ByAddres(BleDeviceData data)
        {
            return delegate(UserControlBleDevice ucBleDevice)
            {
                return (ucBleDevice.Address == data.Address);
            };
        }
    }
}

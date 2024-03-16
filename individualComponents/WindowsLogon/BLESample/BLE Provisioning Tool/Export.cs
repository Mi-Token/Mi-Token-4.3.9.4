using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;

namespace BLE_Provisioning_Tool
{
    class Exporter
    {
        private List<BondInfo> m_bonds = new List<BondInfo>(30);

        public void AddBond(BondInfo bondToAdd)
        {
            BondInfo bondFound = m_bonds.Find(ByAddress(bondToAdd));
            if (bondFound == null)
            {
                // Add new bond
                m_bonds.Add(bondToAdd);
            }
            else
            {   // Update bond
                bondFound = bondToAdd;
            }
        }

        public bool RemoveBond(BondInfo bondToRemove)
        {
            BondInfo bondFound = m_bonds.Find(ByAddress(bondToRemove));
            if (bondFound == null)
            {
                // Could not find bond to remove
                return false;
            }
            else
            {   // Attempt to remove bond
                return m_bonds.Remove(bondFound);
            }
        }

        public bool Update(BondInfo bondToUpdate, BeaconData beaconData, ConfigurationData cfgData)
        {
            bondToUpdate = m_bonds.Find(ByAddress(bondToUpdate));
            if (bondToUpdate != null)
            {
                bondToUpdate.DeviceData.BeaconData = beaconData;
                return true;
            }
            if (cfgData != null)
            {
                bondToUpdate.CfgData = cfgData;
                return true;
            }
            return false;
        }

        public bool IsExisit(BondInfo bondToFind)
        {
            return (m_bonds.Find(ByAddress(bondToFind)) != null);
        }

        public bool GetBond(BondInfo bondToFind, out BondInfo bondFound)
        {
            bondFound = m_bonds.Find(ByAddress(bondToFind));
            return (bondFound != null);
        }

        public string GetStringAll()
        {
            StringBuilder builder = new StringBuilder();
            foreach (BondInfo b in m_bonds)
            {
                builder.AppendLine(b.GetExportString());
            }
            return builder.ToString();
        }

        public string GetStringExtendedAll()
        {
            StringBuilder builder = new StringBuilder();
            foreach (BondInfo b in m_bonds)
            {
                builder.AppendLine(b.GetExportExtendedString());
            }
            return builder.ToString();
        }

        static public Predicate<BondInfo> ByAddress(BondInfo bond)
        {
            return delegate(BondInfo b)
            {
                return b.IsEqual(bond);
            };
        }

        public bool IsEmpty()
        {
            return (m_bonds.Count == 0);
        }
    }

    class BondInfo
    {
        // Member variables
        DateTime m_bondTime;

        // Properties
        public BleDeviceData DeviceData { get; private set; }
        public SharedSecret BondSharedSecret { get; private set; }
        public ConfigurationData CfgData { get; set; }

        // Constructors
        public BondInfo(DeviceAddress address)
        {
            DeviceData = new BleDeviceData();
            DeviceData.Address = address;
        }

        public BondInfo(BleDeviceData deviceData, SharedSecret sharedSecret, DateTime bondTime, ConfigurationData cfgData = null)
        {
            DeviceData = deviceData;
            BondSharedSecret = sharedSecret;
            m_bondTime = bondTime;
            if (cfgData != null)
                CfgData = cfgData;
        }

        // Functions
        public String GetExportString()
        {
            StringBuilder strBuilder = new StringBuilder();
            strBuilder.Append("BLE");
            strBuilder.Append(" ");
            strBuilder.Append(DeviceData.Address.GetRawString());
            strBuilder.Append(" ");
            strBuilder.Append(BondSharedSecret.GetExportString());
            return strBuilder.ToString();
        }

        public String GetExportExtendedString()
        {
            StringBuilder strBuilder = new StringBuilder();;
            strBuilder.Append("Address(Display-Hex)=");
            strBuilder.Append(DeviceData.Address.GetDisplayString() + Environment.NewLine);
            strBuilder.Append("ID(Hex)=");
            strBuilder.Append(DeviceData.Address.GetExportString() + Environment.NewLine);
            strBuilder.Append("UUID(Hex)=");
            strBuilder.Append(DeviceData.BeaconData.UUID.GetString() + Environment.NewLine);
            strBuilder.Append("Major=");
            strBuilder.Append(DeviceData.BeaconData.Major.ToString() + Environment.NewLine);
            strBuilder.Append("Minor=");
            strBuilder.Append(DeviceData.BeaconData.Minor.ToString() + Environment.NewLine);
            strBuilder.Append("SharedSecret(Hex)=");
            strBuilder.Append(BondSharedSecret.GetExportString() + Environment.NewLine);
            strBuilder.Append("TimeOfBond(UTC)=");
            strBuilder.Append(Helper.GetTimestamp(m_bondTime) + Environment.NewLine);
            if (CfgData != null)
            {                
                strBuilder.Append("FirmwareRev=");
                strBuilder.Append(CfgData.FirmwareRevision + Environment.NewLine);
                strBuilder.Append("HardwareRev=");
                strBuilder.Append(CfgData.HardwareRevision + Environment.NewLine);
                if (CfgData.MinimumFirmwareRevision != null)
                {
                    strBuilder.Append("MinimumFirmwareRev=");
                    strBuilder.Append(CfgData.MinimumFirmwareRevision + Environment.NewLine);
                }
                strBuilder.Append("AdvertInterval(ms)=");
                strBuilder.Append(CfgData.AdvertisingIntervalMs.ToString() + Environment.NewLine);
                strBuilder.Append("TransmittingPower(dBM)");
                strBuilder.Append(CfgData.GetTxPowerLevelString() + Environment.NewLine);
                strBuilder.Append("BatteryThreshold(%)=");
                strBuilder.Append(CfgData.GetBatteryThresholdPercentString() + Environment.NewLine);
                strBuilder.Append("MeasuredPowerSettings(-24,-6,0,4)=");
                strBuilder.Append(CfgData.GetMeasuredTxPowerMinus24dBmString() + ", " + CfgData.GetMeasuredTxPowerMinus6dBmString() + ", " + CfgData.GetMeasuredTxPower0dBmString() + ", " + CfgData.GetMeasuredTxPower4dBmString() + Environment.NewLine);
                strBuilder.Append("ConnectionSettings(min,max,latency,timeout)=");
                strBuilder.Append(CfgData.ConnMinIntervalMs.ToString() + ", " + CfgData.ConnMaxIntervalMs.ToString() + ", " + CfgData.ConnLatency.ToString() + ", " + CfgData.ConnTimeoutMs.ToString() + Environment.NewLine);
                if (CfgData.PeripheralsSupported != null)
                {
                    strBuilder.Append("Peripherals(Hex)=");
                    strBuilder.Append(String.Format("{0:X4}", CfgData.PeripheralsSupported.Value));
                }
                strBuilder.Append(Environment.NewLine);
                strBuilder.Append(CfgData.AdvertisingStageSettings.GetExportString());
            }
            return strBuilder.ToString();
        }

        public bool IsEqual(BondInfo bond)
        {
            return DeviceData.Address == bond.DeviceData.Address;
        }
    }
}

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;

namespace BLE_Provisioning_Tool
{
    public partial class UserControlBleDevice : UserControl
    {
        public class UserControlBleDeviceEventArgs : EventArgs
        {
            public UserControlBleDeviceEventArgs(ButtonType buttonType, BleDeviceData data)
            {
                ButtonType = buttonType;
                Data = data;
            }
            public ButtonType ButtonType { get; private set; }
            public BleDeviceData Data { get; private set; }
        }

        public enum ButtonType { eProvisionButton, eUnpairButton, ePingButton, eConfigureButton, eUpdateButton, eRefreshButton };

        public event EventHandler<UserControlBleDeviceEventArgs> ProvisionButtonClicked;
        public event EventHandler<UserControlBleDeviceEventArgs> UnpairButtonClicked;
        public event EventHandler<UserControlBleDeviceEventArgs> PingButtonClicked;
        public event EventHandler<UserControlBleDeviceEventArgs> ConfigureButtonClicked;
        public event EventHandler<UserControlBleDeviceEventArgs> UpdateButtonClicked;
        public event EventHandler<UserControlBleDeviceEventArgs> RefreshButtonClicked;

        private Boolean m_isFormFinialized = false;        

        protected virtual void OnProvisionButtonClicked(BleDeviceData data)
        {
            if (ProvisionButtonClicked != null)
                ProvisionButtonClicked(this, new UserControlBleDeviceEventArgs(ButtonType.eProvisionButton, data));
        }

        protected virtual void OnUnpairButtonClicked(BleDeviceData data)
        {
            if (UnpairButtonClicked != null)
                UnpairButtonClicked(this, new UserControlBleDeviceEventArgs(ButtonType.eUnpairButton, data));
        }

        protected virtual void OnPingButtonClicked(BleDeviceData data)
        {
            if (PingButtonClicked != null)
                PingButtonClicked(this, new UserControlBleDeviceEventArgs(ButtonType.ePingButton, data));
        }

        protected virtual void OnConfigureButtonClicked(BleDeviceData data)
        {
           if (ConfigureButtonClicked != null)
               ConfigureButtonClicked(this, new UserControlBleDeviceEventArgs(ButtonType.eConfigureButton, data));
        }

        protected virtual void OnUpdateButtonClicked(BleDeviceData data)
        {
            if (UpdateButtonClicked != null)
                UpdateButtonClicked(this, new UserControlBleDeviceEventArgs(ButtonType.eUpdateButton, data));
        }

        protected virtual void OnRefreshButtonClicked(BleDeviceData data)
        {
            if (RefreshButtonClicked != null)
                RefreshButtonClicked(this, new UserControlBleDeviceEventArgs(ButtonType.eRefreshButton, data));
        }

        private BleDeviceData m_bleDeviceData;
        private delegate void DelegateUpdateData(BleDeviceData data);
        private delegate void DelegateUpdateRssi(int rssi);
        private delegate void DelegateEnableVisibleButton(Button butt, Boolean isEnabled);

        public UserControlBleDevice(BleDeviceData data)
        {
            InitializeComponent();
            
            // Initialise progress bar
            progressBarRssi.Minimum = 0;
            progressBarRssi.Maximum = 100;

            // Initial update with data
            UpdateData(data);

            // Change text colour
            groupBoxBleDev.ForeColor = Color.FromArgb(1, 74, 156); // Mi-token blue
            foreach (Control c in this.groupBoxBleDev.Controls)
            {
                c.ForeColor = Color.Black;
            }

            // Disable all buttons
            buttonPairCreate.Enabled = false;
            buttonPairRemove.Enabled = false;
            buttonPing.Enabled = false;
            buttonConfigure.Enabled = false;
            buttonUpdate.Enabled = false;

            // Hide all buttons
            buttonPairCreate.Visible = false;
            buttonPairRemove.Visible = false;
            buttonPing.Visible = false;
            buttonConfigure.Visible = false;
            buttonUpdate.Visible = false;            

            // Show label button processing
            labelButtonProcessing.Visible = true;

            // Default label last seen
            labelTimeOfLastPollSeen.Text = "-";

            // Hide this label and only show when in valid use case (beacon/auth-beacon)
            labelProvStatus.Visible = false;

            // Hide button
            buttonRefresh.Visible = false;

            // Enable double buffering
            this.DoubleBuffered = true;
        }

        public void UpdateData(BleDeviceData data)
        {
            // Only retain time of last seen poll if poll value is different
            if (m_bleDeviceData != null)
            {
                if (m_bleDeviceData.LastPollSeen == data.LastPollSeen)
                {
                    data.TimeOfLastPollSeen = m_bleDeviceData.TimeOfLastPollSeen;
                }
#if false // Testing: Used to indicate to user that a new poll was found
                else
                {
                    System.Media.SystemSounds.Beep.Play();
                }
#endif
            }

            // Update member variable
            m_bleDeviceData = data;

            // Populate controls
            groupBoxBleDev.Text = data.DeviceName.ToString();
            labelAddress.Text = data.Address.GetDisplayString();

            // Populate based on whether iBeacon Data is available
            if (data.BeaconData.IsInit)
            {
                labelDevUuid.Text = data.BeaconData.UUID.GetString();
                labelMajor.Text = data.BeaconData.Major.ToString();
                labelMinor.Text = data.BeaconData.Minor.ToString();
            }
            else
            {
                labelDevUuid.Text = "N/A";
                labelMajor.Text = "N/A";
                labelMinor.Text = "N/A";
            }

            UpdateRssi(data.RSSI);

            // Populate last seen label
            TimeSpan ts = DateTime.Now.Subtract(m_bleDeviceData.TimeOfLastPollSeen);
            String timeStr = Math.Round(ts.TotalSeconds).ToString() + " sec ago";
            if (Math.Round(ts.TotalSeconds) == 0)
            {
                timeStr = "Now";
                labelTimeOfLastPollSeen.ForeColor = System.Drawing.Color.Green;
            }
            else if (Math.Round(ts.TotalSeconds) >= 60.0)
            {
                timeStr = "1 min+ ago";
                labelTimeOfLastPollSeen.ForeColor = System.Drawing.Color.Red;
            }
            else
            {
                labelTimeOfLastPollSeen.ForeColor = System.Drawing.Color.Black;
            }
            labelTimeOfLastPollSeen.Text = string.Format("{0}", timeStr);
             

            if (data.IsBonded)
            {
                buttonPairCreate.Enabled = false;
            }
        }

        private void UpdateRssi(byte rssi)
        {
            m_bleDeviceData.RSSI = rssi;
            progressBarRssi.Value = ConvertRssidBmToSignalQuality((sbyte)rssi);
        }

        private static readonly sbyte MIN_RSSI_DBM = -89;
        private static readonly sbyte MAX_RSSI_DBM = -38;
        static public int ConvertRssidBmToSignalQuality(sbyte rssi)
        {
            if ((rssi == 0) || (rssi <= MIN_RSSI_DBM))
            {
                return 0;
            }
            else if (rssi >= MAX_RSSI_DBM)
            {
                return 100;
            }
            else
            {
                return ((rssi - MIN_RSSI_DBM) * 100) / (MAX_RSSI_DBM - MIN_RSSI_DBM);
            }
        }

        public DeviceAddress Address
        {
            get
            {
                return m_bleDeviceData.Address;
            }
        }

        public string DeviceName
        {
            get
            {
                return m_bleDeviceData.DeviceName;
            }
        }        

        public Boolean IsSecureBondSvcExists 
        {
            get
            {
                return m_bleDeviceData.IsSecureBondSvcExists;
            }

            set
            {
                m_bleDeviceData.IsSecureBondSvcExists = value;
            }
        }

        public Boolean IsDevInfoSvcExists 
        {
            get
            {
                return m_bleDeviceData.IsDevInfoSvcExists;
            }

            set
            {
                m_bleDeviceData.IsDevInfoSvcExists = value;
            }
        }

        public Boolean IsSysCfgSvcExists 
        {
            get
            {
                return m_bleDeviceData.IsSysCfgSvcExists;
            }

            set
            {
                m_bleDeviceData.IsSysCfgSvcExists = value;
            }
        }

        public Boolean IsAuthenSvcExists 
        {
            get
            {
                return m_bleDeviceData.IsAuthenSvcExists;
            }

            set
            {
                m_bleDeviceData.IsAuthenSvcExists = value;
            }
        }

        public Boolean IsOadSvcExists 
        {
            get
            {
                return m_bleDeviceData.IsOadSvcExists;
            }

            set
            {
                m_bleDeviceData.IsOadSvcExists = value;
            }
        }

        public Boolean IsBuzzerExists 
        {
            get
            {
                return m_bleDeviceData.IsBuzzerExists;
            }

            set
            {
                m_bleDeviceData.IsBuzzerExists = value;
            }
        }

        public Boolean IsProvisioned 
        {
            get
            {
                return m_bleDeviceData.IsProvisioned;
            }

            set
            {
                m_bleDeviceData.IsProvisioned = value;
            }
        }

        public Boolean IsOadFirmware 
        {
            get
            {
                return m_bleDeviceData.IsOadFirmware;
            }

            set
            {
                m_bleDeviceData.IsOadFirmware = value;
            }
        }

        public ConfigurationData.eUseCase UseCase
        {
            get
            {
                return m_bleDeviceData.UseCase;
            }

            set
            {
                m_bleDeviceData.UseCase = value;
            }
        }

        public Boolean IsAlreadyBonded 
        {
            get
            {
                return m_bleDeviceData.IsAlreadyBonded;
            }

            set
            {
                m_bleDeviceData.IsAlreadyBonded = value;
            }
        }

        private void buttonPairCreate_Click(object sender, EventArgs e)
        {
            OnProvisionButtonClicked(m_bleDeviceData);
        }

        private void buttonPairRemove_Click(object sender, EventArgs e)
        {
            OnUnpairButtonClicked(m_bleDeviceData);
        }

        private void buttonPing_Click(object sender, EventArgs e)
        {
            OnPingButtonClicked(m_bleDeviceData);
        }

        private void buttonConfigure_Click(object sender, EventArgs e)
        {
            OnConfigureButtonClicked(m_bleDeviceData);
        }

        private void buttonUpdate_Click(object sender, EventArgs e)
        {
            OnUpdateButtonClicked(m_bleDeviceData);
        }

        private void buttonRefresh_Click(object sender, EventArgs e)
        {
            OnRefreshButtonClicked(m_bleDeviceData);
        }

        public Button FindButton(ButtonType buttonType)
        {
            switch (buttonType)
            {
                case ButtonType.eProvisionButton:
                    return buttonPairCreate;
                case ButtonType.eUnpairButton:
                    return buttonPairRemove;
                case ButtonType.ePingButton:
                    return buttonPing;
                case ButtonType.eConfigureButton:
                    return buttonConfigure;
                case ButtonType.eUpdateButton:
                    return buttonUpdate;
                case ButtonType.eRefreshButton:
                    return buttonRefresh;
                default:
                    return null;
            }
        }

        private void EnableAndVisible(Button butt, Boolean isEnabled)
        {
            butt.Enabled = isEnabled;
            butt.Visible = true;
        }

        public bool EnableButton(ButtonType buttonType, Boolean isEnabled)
        {
            Button butt = FindButton(buttonType);
            if (butt != null)
            {
                if (butt.InvokeRequired)
                {
                    DelegateEnableVisibleButton d = new DelegateEnableVisibleButton(EnableAndVisible);
                    this.Invoke(d, new object[] { butt, isEnabled });

                }
                else
                {
                    EnableAndVisible(butt, isEnabled);
                }
                return true;
            }
            return false;
        }

        private void ControlFinialized()
        {
            labelButtonProcessing.Visible = false;

            m_isFormFinialized = true;
        }

        public bool IsFormFinialized()
        {
            return m_isFormFinialized;
        }

        public void SetLabelButtonProcessingText(String text)
        {
            labelButtonProcessing.Text = text;
        }

        private void SetLabelProvisionStatus(bool isTrigger)
        {
            switch (UseCase)
            {
                case ConfigurationData.eUseCase.eAuthBeacon:
                case ConfigurationData.eUseCase.eBeacon:
                case ConfigurationData.eUseCase.eBlu:
                    labelProvStatus.Visible = true;
                    labelProvStatus.Text = isTrigger ? "<Provisioned>" : "<Unprovisioned>";
                    break;
            }
        }

        public void RefreshServiceControls()
        {
            Debug.WriteLine(String.Format("{0}:RefreshServiceControls:IsSecureBondSvcExists=[{1}] IsDevInfoSvcExists=[{2}] IsSysCfgSvcExists=[{3}] IsAuthenSvcExists=[{4}] IsOadSvcExists=[{5}] IsBuzzerExists=[{6}] IsProvisioned=[{7}] IsOadFirmware=[{8}] UseCase=[{9}] IsAlreadyBonded=[{10}]",
                DateTime.Now, IsSecureBondSvcExists, IsDevInfoSvcExists, IsSysCfgSvcExists, IsAuthenSvcExists, IsOadSvcExists, IsBuzzerExists, IsProvisioned, IsOadFirmware, UseCase, IsAlreadyBonded));

            EnableButton(ButtonType.eProvisionButton, false);
            EnableButton(ButtonType.eUnpairButton, false);
            EnableButton(ButtonType.eConfigureButton, false);
            EnableButton(ButtonType.ePingButton, false);
            EnableButton(ButtonType.eUpdateButton, false);
            if (IsSysCfgSvcExists)
            {
                if (IsDevInfoSvcExists || IsSecureBondSvcExists || IsAuthenSvcExists)
                {
                    if (IsDevInfoSvcExists && IsSecureBondSvcExists)
                    {
                        if ((UseCase == ConfigurationData.eUseCase.eBeacon) || (UseCase == ConfigurationData.eUseCase.eAuthBeacon))
                        {
                            if (IsAuthenSvcExists)
                            {
                                if (!IsAlreadyBonded)
                                {
                                    EnableButton(ButtonType.eProvisionButton, true);
                                }
                            }
                            // Update Provisioning Status Label
                            SetLabelProvisionStatus(IsProvisioned);
                        }
                        else if (UseCase == ConfigurationData.eUseCase.eBlu)
                        {
                            if (!IsAlreadyBonded)
                            {
                                EnableButton(ButtonType.eProvisionButton, true);
                            }
                            // Update Provisioning Status Label
                            SetLabelProvisionStatus(IsAlreadyBonded);
                        }

                        EnableButton(ButtonType.eUnpairButton, true);
                    }
                    if (IsDevInfoSvcExists)
                    {
                        if (IsAuthenSvcExists)
                        {
                            EnableButton(ButtonType.eConfigureButton, true);
                        }
                    }
                    if (IsSecureBondSvcExists && IsBuzzerExists)
                    {
                        EnableButton(ButtonType.ePingButton, true);
                    }
                }
            }
            if (UseCase == ConfigurationData.eUseCase.eBoot)
            {
                EnableButton(ButtonType.eConfigureButton, true);
            }
            if (IsOadSvcExists)
            {
                EnableButton(ButtonType.eUpdateButton, true);
            }

            ControlFinialized();
        }
    }
}
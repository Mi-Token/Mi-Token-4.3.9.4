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
    public partial class UserControlConfigurationData : UserControl
    {
        public class UserControlConfigurationDataEventArgs : EventArgs
        {
            public UserControlConfigurationDataEventArgs(ConfigurationData oldData, ConfigurationData newData)
            {
                OldData = oldData;
                NewData = newData;
            }
            public ConfigurationData OldData { get; private set; }
            public ConfigurationData NewData { get; private set; }
        }

        public event EventHandler BackButtonClicked;
        public event EventHandler<UserControlConfigurationDataEventArgs> ApplyButtonClicked;        

        protected virtual void OnBackButtonClicked()
        {
            if (BackButtonClicked != null)
                BackButtonClicked(this, null);
        }

        protected virtual void OnApplyButtonClicked(ConfigurationData oldData, ConfigurationData newData)
        {
            if (ApplyButtonClicked != null)
                ApplyButtonClicked(this, new UserControlConfigurationDataEventArgs(oldData, newData));
        }
        
        private ConfigurationData m_cfgData;
        private ConfigurationData m_originalCfgData;

        public ConfigurationData OriginalCfgData
        {
            get
            {
                return m_originalCfgData;
            }
        }

        public UserControlConfigurationData()
        {
            InitializeComponent();
            this.DoubleBuffered = true;            
        }

        public void Init(ConfigurationData cfgData, bool isUseCaseBoot)
        {
            comboBoxDevSettingsTxPowerDbm.Items.Clear();
            if (!isUseCaseBoot)
            {
                // Populate combobox with power levels based on device type                   
                ConfigurationData.eDeviceType deviceType = cfgData.GetDeviceType();
                int[] txPowerLevels = (int[])Enum.GetValues(typeof(ConfigurationData.eTxPowerLevel));
                foreach (int level in txPowerLevels)
                {
                    // Ensure that for CC2541 devices will not allow to populate with particular TX power levels
                    if ((level != (int)ConfigurationData.eTxPowerLevel.eInvalid) && ((deviceType == ConfigurationData.eDeviceType.eCC2541 && level != (int)ConfigurationData.eTxPowerLevel.e4dBm) || deviceType == ConfigurationData.eDeviceType.eCC2540))
                    {
                        comboBoxDevSettingsTxPowerDbm.Items.Add(level.ToString());
                    }
                }

                // Select default value if available
                try
                {
                    comboBoxDevSettingsTxPowerDbm.SelectedIndex = 2;
                }
                catch
                {
                    Debug.WriteLine(String.Format("Tx Power Level not available"));
                }
            }

            cfgData.IsUpdateRequired = false; // Reset this as initialised
            m_originalCfgData = new ConfigurationData(cfgData);            
            UpdateCfgData(ref cfgData, isUseCaseBoot);
        }

        private void UpdateCfgData(ref ConfigurationData cfgData, bool isUseCaseBoot)
        {
            m_cfgData = new ConfigurationData(cfgData); // Ensure new copy is created

            // Assign text to controls 
            textBoxDevInfoAddress.Text = cfgData.Address.GetDisplayString();
            textBoxDevInfoSysId.Text = Helper.GetSystemIdString(cfgData.SystemId);
            textBoxDevInfoFwRev.Text = cfgData.FirmwareRevision;
            textBoxDevInfoHwRev.Text = cfgData.HardwareRevision;
            textBoxDevSettingsName.Text = cfgData.DeviceName;
            if (!isUseCaseBoot)
            {
                if (cfgData.MinimumFirmwareRevision != null)
                {
                    textBoxDevInfoMinFwRev.Text = cfgData.MinimumFirmwareRevision;
                }
                else
                {
                    textBoxDevInfoMinFwRev.Text = "";
                }
                comboBoxDevSettingsTxPowerDbm.Text = cfgData.GetTxPowerLevelString();
                textBoxDevSettingsBattThresPercent.Text = cfgData.GetBatteryThresholdPercentString();
                textBoxConnMinIntervalMs.Text = cfgData.ConnMinIntervalMs.ToString();
                textBoxConnMaxIntervalMs.Text = cfgData.ConnMaxIntervalMs.ToString();
                textBoxConnLatency.Text = cfgData.ConnLatency.ToString();
                textBoxConnTimeoutMs.Text = cfgData.ConnTimeoutMs.ToString();
                textBoxIBeaconDevUuid.Text = cfgData.UUID.GetString();
                textBoxIBeaconMajor.Text = cfgData.Major.ToString();
                textBoxIBeaconMinor.Text = cfgData.Minor.ToString();
                textBoxMeasuredTxPowerMinus24.Text = cfgData.GetMeasuredTxPowerMinus24dBmString();
                textBoxMeasuredTxPowerMinus6.Text = cfgData.GetMeasuredTxPowerMinus6dBmString();
                textBoxMeasuredTxPower0.Text = cfgData.GetMeasuredTxPower0dBmString();
                textBoxMeasuredTxPower4.Text = cfgData.GetMeasuredTxPower4dBmString();
                if (cfgData.PeripheralsSupported == null)
                {
                    groupBoxPeripherals.Visible = false;
                }
                else
                {
                    richTextBoxPeripherals.Text = cfgData.PeripheralsSupported.ToPrintString();
                }

                var fmRev = cfgData.GetFirmwareRevision();
                if (fmRev.Major <= 1 && fmRev.Minor <= 11)
                {
                    groupBoxAdvertStagesBcn.Visible = false;
                    groupBoxAdvertStagesBlu.Visible = false;
                    groupBoxAdvertStagesLegacy.Visible = true;
                    textBoxAdvertStageIntervalLegacy.Text = cfgData.AdvertisingIntervalMs.ToString();
                }
                else
                {
                    if ((cfgData.GetUseCase() == ConfigurationData.eUseCase.eAuthBeacon) || (cfgData.GetUseCase() == ConfigurationData.eUseCase.eBeacon))
                    {
                        groupBoxAdvertStagesBcn.Visible = true;
                        groupBoxAdvertStagesBlu.Visible = false;
                        groupBoxAdvertStagesLegacy.Visible = false;
                        textBoxAdvertStage0IntervalBcn.Text = cfgData.AdvertisingStageSettings.GetIntervalMsStr(ConfigurationData.AdvSmStageSettings.Stages.Stage0);
                        textBoxAdvertStage1IntervalBcn.Text = cfgData.AdvertisingStageSettings.GetIntervalMsStr(ConfigurationData.AdvSmStageSettings.Stages.Stage1);
                    }
                    else
                    {
                        groupBoxAdvertStagesBcn.Visible = false;
                        groupBoxAdvertStagesBlu.Visible = true;
                        groupBoxAdvertStagesLegacy.Visible = false;
                        textBoxAdvertStage0Timeout.Text = cfgData.AdvertisingStageSettings.GetIdleTimeoutMsStr(ConfigurationData.AdvSmStageSettings.Stages.Stage0);
                        textBoxAdvertStage0Interval.Text = cfgData.AdvertisingStageSettings.GetIntervalMsStr(ConfigurationData.AdvSmStageSettings.Stages.Stage0);
                        textBoxAdvertStage1Timeout.Text = cfgData.AdvertisingStageSettings.GetIdleTimeoutMsStr(ConfigurationData.AdvSmStageSettings.Stages.Stage1);
                        textBoxAdvertStage1Interval.Text = cfgData.AdvertisingStageSettings.GetIntervalMsStr(ConfigurationData.AdvSmStageSettings.Stages.Stage1);
                        textBoxAdvertStage2Timeout.Text = cfgData.AdvertisingStageSettings.GetIdleTimeoutMsStr(ConfigurationData.AdvSmStageSettings.Stages.Stage2);
                        textBoxAdvertStage2Interval.Text = cfgData.AdvertisingStageSettings.GetIntervalMsStr(ConfigurationData.AdvSmStageSettings.Stages.Stage2);
                        textBoxAdvertStage3Timeout.Text = cfgData.AdvertisingStageSettings.GetIdleTimeoutMsStr(ConfigurationData.AdvSmStageSettings.Stages.Stage3);
                        textBoxAdvertStage3Interval.Text = cfgData.AdvertisingStageSettings.GetIntervalMsStr(ConfigurationData.AdvSmStageSettings.Stages.Stage3);
                    }
                }
            }
            else
            {
                textBoxDevInfoMinFwRev.Text = "-";
                comboBoxDevSettingsTxPowerDbm.Text = "-";
                textBoxDevSettingsBattThresPercent.Text = "-";
                textBoxConnMinIntervalMs.Text = "-";
                textBoxConnMaxIntervalMs.Text = "-";
                textBoxConnLatency.Text = "-";
                textBoxConnTimeoutMs.Text = "-";
                textBoxIBeaconDevUuid.Text = "-";
                textBoxIBeaconMajor.Text = "-";
                textBoxIBeaconMinor.Text = "-";
                textBoxMeasuredTxPowerMinus24.Text = "-";
                textBoxMeasuredTxPowerMinus6.Text = "-";
                textBoxMeasuredTxPower0.Text = "-";
                textBoxMeasuredTxPower4.Text = "-";
                richTextBoxPeripherals.Text = "-";
                textBoxAdvertStage0Timeout.Text = "-";
                textBoxAdvertStage0Interval.Text = "-";
                textBoxAdvertStage1Timeout.Text = "-";
                textBoxAdvertStage1Interval.Text = "-";
                textBoxAdvertStage2Timeout.Text = "-";
                textBoxAdvertStage2Interval.Text = "-";
                textBoxAdvertStage3Timeout.Text = "-";
                textBoxAdvertStage3Interval.Text = "-";
                textBoxAdvertStage0IntervalBcn.Text = "-";
                textBoxAdvertStage1IntervalBcn.Text = "-";
                groupBoxAdvertStagesBcn.Visible = false;
                groupBoxAdvertStagesBlu.Visible = false;
                textBoxAdvertStageIntervalLegacy.Visible = false;
            }

            // Only allow configuration on bonded device after a particular revision
            bool bEnableButtons = false;
            StringBuilder stringBuilder = new StringBuilder();
            stringBuilder.Append("Note: ");
            if (!isUseCaseBoot)
            {
                ConfigurationData.Revision_t rev = cfgData.GetFirmwareRevision();                
                if ((rev.Major >= ConfigurationData.ALLOW_CFG_SET_REVISION_MAJOR) && (rev.Minor >= ConfigurationData.ALLOW_CFG_SET_REVISION_MINOR))
                {
                    bEnableButtons = cfgData.IsBonded;

                    if (cfgData.IsBonded)
                    {
                        var useCase = cfgData.GetUseCase();
                        if ((useCase == ConfigurationData.eUseCase.eBeacon) || (useCase == ConfigurationData.eUseCase.eAuthBeacon))
                        {
                            if (cfgData.IsProvisioned)
                            {
                                stringBuilder.Append("Provisioning has been finialized.");
                            }
                            else
                            {
                                stringBuilder.Append("Provisioning has NOT been finialized.");
                            }
                        }
                        else
                        {
                            stringBuilder.Append("Device can now be configured.");
                        }
                    }
                    else
                    {
                        stringBuilder.Append("Configurable once provisioned.");
                    }
                }
                else
                {
                    if (cfgData.IsBonded)
                    {
                        stringBuilder.Append(string.Format("Configurable for Firmware Revisions V{0}.{1}+", ConfigurationData.ALLOW_CFG_SET_REVISION_MAJOR, ConfigurationData.ALLOW_CFG_SET_REVISION_MINOR));
                    }
                    else
                    {
                        stringBuilder.Append("Configurable once provisioned.");
                    }
                }
                
            }
            else
            {
                stringBuilder.Append("Cannot be configured.");
            }
            labelSpecialNote.Text = stringBuilder.ToString();

            // Enable or disable controls
            if (isUseCaseBoot) 
            {
                bEnableButtons = false;
            }

            buttonCfgApply.Enabled = bEnableButtons;
            buttonReset.Enabled = bEnableButtons;
            buttonCfgBack.Enabled = true;
            textBoxDevInfoAddress.Enabled = bEnableButtons;
            textBoxDevInfoSysId.Enabled = bEnableButtons;
            textBoxDevInfoFwRev.Enabled = bEnableButtons;
            textBoxDevInfoHwRev.Enabled = bEnableButtons;
            textBoxDevInfoMinFwRev.Enabled = bEnableButtons;
            textBoxDevSettingsName.Enabled = bEnableButtons;
            if (cfgData.TxPowerDbm == ConfigurationData.eTxPowerLevel.eInvalid)
            {
                comboBoxDevSettingsTxPowerDbm.Enabled = false;
            }
            else
            {
                comboBoxDevSettingsTxPowerDbm.Enabled = bEnableButtons;
            }
            if ((cfgData.BatteryThresholdPercent != -1) && (cfgData.BatteryThresholdPercent != 0))
            {
                textBoxDevSettingsBattThresPercent.Enabled = bEnableButtons;
            }
            else
            {
                textBoxDevSettingsBattThresPercent.Enabled = false;
            }
            textBoxConnMinIntervalMs.Enabled = bEnableButtons;
            textBoxConnMaxIntervalMs.Enabled = bEnableButtons;
            textBoxConnLatency.Enabled = bEnableButtons;
            textBoxConnTimeoutMs.Enabled = bEnableButtons;
            textBoxIBeaconDevUuid.Enabled = bEnableButtons;
            textBoxIBeaconMajor.Enabled = bEnableButtons;
            textBoxIBeaconMinor.Enabled = bEnableButtons;
            textBoxMeasuredTxPowerMinus24.Enabled = bEnableButtons;
            textBoxMeasuredTxPowerMinus6.Enabled = bEnableButtons;
            textBoxMeasuredTxPower0.Enabled = bEnableButtons;
            if (cfgData.GetDeviceType() == ConfigurationData.eDeviceType.eCC2540)
            {
                textBoxMeasuredTxPower4.Enabled = bEnableButtons;
            }
            else
            {
                textBoxMeasuredTxPower4.Enabled = false;
            }
            richTextBoxPeripherals.Enabled = bEnableButtons;
            textBoxAdvertStage0Timeout.Enabled = bEnableButtons;
            textBoxAdvertStage0Interval.Enabled = bEnableButtons;
            textBoxAdvertStage1Timeout.Enabled = bEnableButtons;
            textBoxAdvertStage1Interval.Enabled = bEnableButtons;
            textBoxAdvertStage2Timeout.Enabled = bEnableButtons;
            textBoxAdvertStage2Interval.Enabled = bEnableButtons;
            textBoxAdvertStage3Timeout.Enabled = bEnableButtons;
            textBoxAdvertStage3Interval.Enabled = bEnableButtons;
            textBoxAdvertStage0IntervalBcn.Enabled = bEnableButtons;
            textBoxAdvertStage1IntervalBcn.Enabled = bEnableButtons;
            textBoxAdvertStageIntervalLegacy.Enabled = bEnableButtons;

            // Set initial focus
            buttonCfgBack.Focus();
        }

        private void buttonCfgBack_Click(object sender, EventArgs e)
        {
            // Trigger event
            OnBackButtonClicked();            
        }

        private void buttonCfgReset_Click(object sender, EventArgs e)
        {
            if (MessageBoxEx.Show("Reset all changes?\nAll changes will be lost", "Configuration Data", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning) == DialogResult.OK)
            {
                UpdateCfgData(ref m_originalCfgData, false);
                this.buttonCfgApply.Enabled = false;
            }
        }

        private void buttonCfgApply_Click(object sender, EventArgs e)
        {
            ConfigurationData.eUseCase useCase = m_cfgData.GetUseCase();
            if ((useCase == ConfigurationData.eUseCase.eBeacon) || (useCase == ConfigurationData.eUseCase.eAuthBeacon))
            {
                DialogResult result = MessageBoxEx.Show("Would you like to finialize your changes?\nSelect 'Yes' to finialize and apply changes.\nSelect 'No' to Apply changes only.", "Confirm", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Question);
                if (result == DialogResult.Yes)
                {
                    m_cfgData.IsProvisioned = true;
                }
                else if (result == DialogResult.No)
                {
                    m_cfgData.IsProvisioned = false;
                }
                else
                {
                    // cancel request                
                    return;
                }
            }
            else
            {
                if (MessageBoxEx.Show("Select 'OK' to apply all changes.", "Confirm", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning) != DialogResult.OK)
                {
                    // cancel request
                    return;
                }
            }
                
            // Trigger event
            OnApplyButtonClicked(m_originalCfgData, m_cfgData);
        }

        private void EnableApplyButton()
        {
#if false
            // Only enable if configuration values have changed
            if (m_cfgData.IsUpdateRequired)
            {
                this.buttonCfgApply.Enabled = true;
            }
            else
            {
                this.buttonCfgApply.Enabled = false;
            }
#else
            this.buttonCfgApply.Enabled = true;
#endif
        }

        private void ErrorMessage(string errorStr)
        {
            MessageBoxEx.Show(errorStr, "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
        }

        public void Reset()
        {
            m_cfgData = null;
            m_originalCfgData = null;
        }

        private void textBoxDevSettingsName_Validating(object sender, CancelEventArgs e)
        {
            try
            {
                // Ensure char array is appropriate size (i.e. UTF-8 8-bit friendly)
                char[] charArray = ((Control)sender).Text.ToCharArray();
                foreach (char charElem in charArray)
                {
                    if (charElem > 0xff)
                    {
                        throw new System.Exception(string.Format("{0}:\nUTF-8 8-bit format only", "Device Name"));
                    }
                }

                // Warning if prefix is changed
                if (!((Control)sender).Text.StartsWith(DevNameFilterType.getTypeStr(DevNameFilterType.eType.MtAll)))
                {
                    MessageBox.Show(string.Format("Unless prefix of \"{0}\" is set, device will not be configurable by this tool.", DevNameFilterType.getTypeStr(DevNameFilterType.eType.MtAll)), "Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                }

                // Ensure length is within bounds                
                if (((Control)sender).Text.Length < ConfigurationData.MIN_DEV_NAME_LEN)
                {
                    throw new System.Exception(string.Format("{0}:\nLength cannot be less than {1}", labelDevSettingsName.Text, ConfigurationData.MIN_DEV_NAME_LEN));
                }
                else if (((Control)sender).Text.Length > ConfigurationData.MAX_DEV_NAME_LEN)
                {
                    throw new System.Exception(string.Format("{0}:\nLength cannot be greater than {1}", labelDevSettingsName.Text, ConfigurationData.MAX_DEV_NAME_LEN));
                }
                else
                {
                    // Remove leading and trailing whitespace
                    ((Control)sender).Text.Trim();

                    // Save value
                    m_cfgData.DeviceName = ((Control)sender).Text;
                }
            }
            catch (Exception exception)
            {                
                // Change text to red
                ((Control)sender).ForeColor = System.Drawing.Color.Red;

                // Cancel validation event
                e.Cancel = true;

                // Display error message
                ErrorMessage(exception.Message);

                // Reset to original value
                if (m_originalCfgData != null)
                {
                    ((Control)sender).ForeColor = System.Drawing.Color.Black;
                    ((Control)sender).Text = m_originalCfgData.DeviceName;                    
                }
            }
        }

        private void textBoxDevSettingsName_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();            
        }

        private void comboBoxDevSettingsTxPowerDbm_Validating(object sender, CancelEventArgs e)
        {
            try
            {
                // Attempt to convert power level and save value
                m_cfgData.TxPowerDbm = (ConfigurationData.eTxPowerLevel)Enum.Parse(typeof(ConfigurationData.eTxPowerLevel), ((Control)sender).Text);
            }
            catch (Exception exception)
            {
                // Change text to red
                ((Control)sender).ForeColor = System.Drawing.Color.Red;

                // Cancel validation event
                e.Cancel = true;

                // Display error message
                ErrorMessage(exception.Message);

                // Reset to original value
                if (m_originalCfgData != null)
                {
                    try
                    {
                        ((Control)sender).ForeColor = System.Drawing.Color.Black;
                        ((Control)sender).Text = Enum.Format(typeof(ConfigurationData.eTxPowerLevel), m_originalCfgData.TxPowerDbm, "d");                        
                    }
                    catch
                    {
                        Debug.WriteLine(String.Format("Could not set TX Power Level"));
                    }
                }
            }
        }

        private void comboBoxDevSettingsTxPowerDbm_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();            
        }

        private void textBoxDevSettingsBattThresPercent_Validating(object sender, CancelEventArgs e)
        {
            try
            {
                // Attempt to convert to decimal
                int val = Convert.ToUInt16(((Control)sender).Text);

                // Ensure value is within bounds
                if (val < ConfigurationData.MIN_BATTERY_THRESHOLD_PERCENT)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be less than {1}", labelDevSettingsBattThresPercent.Text, ConfigurationData.MIN_BATTERY_THRESHOLD_PERCENT));
                }
                else if (val > ConfigurationData.MAX_BATTERY_THRESHOLD_PERCENT)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be greater than {1}", labelDevSettingsBattThresPercent.Text, ConfigurationData.MAX_BATTERY_THRESHOLD_PERCENT));
                }
                else
                {
                    // Save Value
                    m_cfgData.BatteryThresholdPercent = val;
                }
            }
            catch (Exception exception)
            {
                // Change text to red
                ((Control)sender).ForeColor = System.Drawing.Color.Red;

                // Cancel validation event
                e.Cancel = true;

                // Display error message
                ErrorMessage(exception.Message);

                // Reset to original value
                if (m_originalCfgData != null)
                {
                   ((Control)sender).ForeColor = System.Drawing.Color.Black;
                   ((Control)sender).Text = m_originalCfgData.BatteryThresholdPercent.ToString();                   
                }
            }
        }

        private void textBoxDevSettingsBattThresPercent_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();  
        }

        private void textBoxConnMinIntervalMs_Validating(object sender, CancelEventArgs e)
        {
            try
            {
                // Attempt to convert to decimal
                int val = Convert.ToUInt16(((Control)sender).Text);

                // Ensure value is within bounds
                if (val < ConfigurationData.MIN_CONN_INTERVAL_MS)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be less than {1}", labelConnMinIntervalMs.Text, ConfigurationData.MIN_CONN_INTERVAL_MS));
                }
                else if (val > ConfigurationData.MAX_CONN_INTERVAL_MS)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be greater than {1}", labelConnMinIntervalMs.Text, ConfigurationData.MAX_CONN_INTERVAL_MS));
                }
                else if (val > m_cfgData.ConnMaxIntervalMs)
                {
                    throw new System.Exception(string.Format("{0}:\nCannot be longer than maximum connection interval of {1} ms", labelConnMinIntervalMs.Text, m_cfgData.ConnMaxIntervalMs));
                }
                else
                {
                    // Save value
                    m_cfgData.ConnMinIntervalMs = val;
                }
            }
            catch (Exception exception)
            {
                // Change text to red
                ((Control)sender).ForeColor = System.Drawing.Color.Red;

                // Cancel validation event
                e.Cancel = true;

                // Display error message
                ErrorMessage(exception.Message);

                // Reset to original value
                if (m_originalCfgData != null)
                {
                    ((Control)sender).ForeColor = System.Drawing.Color.Black;
                    ((Control)sender).Text = m_originalCfgData.ConnMinIntervalMs.ToString();                    
                }
            }
        }

        private void textBoxConnMinIntervalMs_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();  
        }

        private void textBoxConnMaxIntervalMs_Validating(object sender, CancelEventArgs e)
        {
            try
            {
                // Attempt to convert to decimal
                int val = Convert.ToUInt16(((Control)sender).Text);

                // Ensure value within bounds
                if (val < ConfigurationData.MIN_CONN_INTERVAL_MS)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be less than {1}", labelConnMaxIntervalMs.Text, ConfigurationData.MIN_CONN_INTERVAL_MS));
                }
                else if (val > ConfigurationData.MAX_CONN_INTERVAL_MS)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be greater than {1}", labelConnMaxIntervalMs.Text, ConfigurationData.MAX_CONN_INTERVAL_MS));
                }
                else if (val < m_cfgData.ConnMinIntervalMs)
                {
                    throw new System.Exception(string.Format("{0}:\nCannot be shorter than minimum connection interval of {1} ms", labelConnMaxIntervalMs.Text, m_cfgData.ConnMinIntervalMs));
                }
                else
                {
                    // Save value
                    m_cfgData.ConnMaxIntervalMs = val;
                }
            }
            catch (Exception exception)
            {
                // Change text to red
                ((Control)sender).ForeColor = System.Drawing.Color.Red;

                // Cancel validation event
                e.Cancel = true;

                // Display error message
                ErrorMessage(exception.Message);

                // Reset to original value
                if (m_originalCfgData != null)
                {
                    ((Control)sender).ForeColor = System.Drawing.Color.Black;
                    ((Control)sender).Text = m_originalCfgData.ConnMaxIntervalMs.ToString();                    
                }
            }
        }

        private void textBoxConnMaxIntervalMs_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();  
        }

        private void textBoxConnLatency_Validating(object sender, CancelEventArgs e)
        {
            try
            {
                // Attempt to convert to decimal
                int val = Convert.ToUInt16(((Control)sender).Text);

                // Ensure value within bounds
                if (val < ConfigurationData.MIN_CONN_LATENCY)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be less than {1}", labelConnLatency.Text, ConfigurationData.MIN_CONN_LATENCY));
                }
                else if (val > ConfigurationData.MAX_CONN_LATENCY)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be greater than {1}", labelConnLatency.Text, ConfigurationData.MAX_CONN_LATENCY));
                }
                else
                {
                    // Save value
                    m_cfgData.ConnLatency = val;
                }
            }
            catch (Exception exception)
            {
                // Change text to red
                ((Control)sender).ForeColor = System.Drawing.Color.Red;

                // Cancel validation event
                e.Cancel = true;

                // Display error message
                ErrorMessage(exception.Message);

                // Reset to original value
                if (m_originalCfgData != null)
                {
                    ((Control)sender).ForeColor = System.Drawing.Color.Black;
                    ((Control)sender).Text = m_originalCfgData.ConnLatency.ToString();                    
                }
            }
        }

        private void textBoxConnLatency_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();  
        }

        private void textBoxConnTimeoutMs_Validating(object sender, CancelEventArgs e)
        {
            try
            {
                // Attempt to convert to decimal
                int val = Convert.ToUInt16(((Control)sender).Text);

                // Ensure value within bounds
                if (val < ConfigurationData.MIN_CONN_TIMEOUT_MS)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be less than {1}", labelConnTimeoutMs.Text, ConfigurationData.MIN_CONN_TIMEOUT_MS));
                }
                else if (val > ConfigurationData.MAX_CONN_TIMEOUT_MS)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be greater than {1}", labelConnTimeoutMs.Text, ConfigurationData.MAX_CONN_TIMEOUT_MS));
                }
                else
                {
                    // Save value
                    m_cfgData.ConnTimeoutMs = val;
                }
            }
            catch (Exception exception)
            {
                // Change text to red
                ((Control)sender).ForeColor = System.Drawing.Color.Red;

                // Cancel validation event
                e.Cancel = true;

                // Display error message
                ErrorMessage(exception.Message);

                // Reset to original value
                if (m_originalCfgData != null)
                {
                    ((Control)sender).ForeColor = System.Drawing.Color.Black;
                    ((Control)sender).Text = m_originalCfgData.ConnTimeoutMs.ToString();                    
                }
            }
        }

        private void textBoxConnTimeoutMs_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();  
        }

        private void textBoxIBeaconDevUuid_Validating(object sender, CancelEventArgs e)
        {            
            try
            {
                string originalStr = ((Control)sender).Text;

                // Ensure correct length
                if (originalStr.Length != Constants.DEVICE_UUID_STRING_LEN)
                {
                    throw new System.Exception(string.Format("{0}:\nLength must be {1}", labeliBeaconDevUuid.Text, Constants.DEVICE_UUID_STRING_LEN));
                }

                // Ensure markers in correct position
                int[] delimMarkers = { 8, 13, 18, 23 };
                foreach (int marker in delimMarkers)
                {
                    if (originalStr.ElementAt(marker) != '-')
                    {
                        throw new System.Exception(string.Format("{0}:\nIncorrect format.\nFormat: XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX", labeliBeaconDevUuid.Text));
                    }
                }

                // Remove deliminators and rebuild string
                string[] splitStr = originalStr.Split('-');
                StringBuilder builder1 = new StringBuilder();
                foreach (string split in splitStr)
                {
                    builder1.Append(split);
                }

                // Seperate into hex values
                string noDelimString = builder1.ToString();
                DeviceUUID hexVals = new DeviceUUID();
                for (int i = 0; i < hexVals.Length; ++i)
                {
                    // Covert to hex
                    hexVals.Value[i] = Convert.ToByte(noDelimString.Substring(i*2, 2), 16);
                }

                // Save value
                m_cfgData.UUID = hexVals;
            }
            catch (Exception exception)
            {
                // Change text to red
                ((Control)sender).ForeColor = System.Drawing.Color.Red;

                // Cancel validation event
                e.Cancel = true;

                // Display error message
                ErrorMessage(exception.Message);

                // Reset to original value
                if (m_originalCfgData != null)
                {
                    ((Control)sender).ForeColor = System.Drawing.Color.Black;
                    ((Control)sender).Text = m_originalCfgData.UUID.GetString();                    
                }
            }
        }

        private void textBoxIBeaconDevUuid_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();  
        }

        private void textBoxIBeaconMajor_Validating(object sender, CancelEventArgs e)
        {
            try
            {
                // Attempt to convert to decimal
                UInt16 val = Convert.ToUInt16(((Control)sender).Text);

                // Ensure value within bounds
                if (val < ConfigurationData.MIN_MAJOR)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be less than {1}", labeliBeaconMajor.Text, ConfigurationData.MIN_MAJOR));
                }
                else if (val > ConfigurationData.MAX_MAJOR)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be greater than {1}", labeliBeaconMajor.Text, ConfigurationData.MAX_MAJOR));
                }
                else
                {
                    // Save Value
                    m_cfgData.Major = val;
                }
            }
            catch (Exception exception)
            {
                // Change text to red
                ((Control)sender).ForeColor = System.Drawing.Color.Red;

                // Cancel validation event
                e.Cancel = true;

                // Display error message
                ErrorMessage(exception.Message);

                // Reset to original value
                if (m_originalCfgData != null)
                {
                    ((Control)sender).ForeColor = System.Drawing.Color.Black;
                    ((Control)sender).Text = m_originalCfgData.Major.ToString();
                }
            }
        }

        private void textBoxIBeaconMajor_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();  
        }

        private void textBoxIBeaconMinor_Validating(object sender, CancelEventArgs e)
        {
            try
            {
                // Attempt to convert to decimal
                UInt16 val = Convert.ToUInt16(((Control)sender).Text);

                // Ensure value within bounds
                if (val < ConfigurationData.MIN_MINOR)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be less than {1}", labeliBeaconMinor.Text, ConfigurationData.MIN_MINOR));
                }
                else if (val > ConfigurationData.MAX_MINOR)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be greater than {1}", labeliBeaconMinor.Text, ConfigurationData.MAX_MINOR));
                }
                else
                {
                    // Save value
                    m_cfgData.Minor = val;
                }
            }            
            catch (Exception exception)
            {
                // Change text to red
                ((Control)sender).ForeColor = System.Drawing.Color.Red;

                // Cancel validation event
                e.Cancel = true;

                // Display error message
                ErrorMessage(exception.Message);

                // Reset to original value
                if (m_originalCfgData != null)
                {
                    ((Control)sender).ForeColor = System.Drawing.Color.Black;
                    ((Control)sender).Text = m_originalCfgData.Minor.ToString();
                }
            }
        }

        private void textBoxIBeaconMinor_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();  
        }

        private void UserControlConfigurationData_Click(object sender, EventArgs e)
        {
            SendKeys.Send("{TAB}");
        }

        private void textBoxMeasuredTxPowerMinus24_Validating(object sender, CancelEventArgs e)
        {
            try
            {                
                // Attempt to convert to integer
                int val = Convert.ToInt32(((Control)sender).Text);

                // Ensure value within bounds
                if (val < 0)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be less than {1}", labelMeasuredTxPowerMinus24.Text, 0));
                }
                else if (val > 128)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be greater than {1}", labelMeasuredTxPowerMinus24.Text, 128));
                }
                else
                {
                    // Save value
                    m_cfgData.MeasuredTxPowerMinus24dBm = Helper.ConvertToTwoComplement(val);
                }
            }
            catch (Exception exception)
            {
                // Change text to red
                ((Control)sender).ForeColor = System.Drawing.Color.Red;

                // Cancel validation event
                e.Cancel = true;

                // Display error message
                ErrorMessage(exception.Message);

                // Reset to original value
                if (m_originalCfgData != null)
                {
                    ((Control)sender).ForeColor = System.Drawing.Color.Black;
                    ((Control)sender).Text = Helper.ConvertFromTwoComplement(m_originalCfgData.MeasuredTxPowerMinus24dBm).ToString();
                }
            }
        }

        private void textBoxMeasuredTxPowerMinus24_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();  
        }

        private void textBoxMeasuredTxPowerMinus6_Validating(object sender, CancelEventArgs e)
        {
            try
            {
                // Attempt to convert to integer
                int val = Convert.ToInt32(((Control)sender).Text);

                // Ensure value within bounds
                if (val < 0)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be less than {1}", labelMeasuredTxPowerMinus6.Text, 0));
                }
                else if (val > 128)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be greater than {1}", labelMeasuredTxPowerMinus6.Text, 128));
                }
                else
                {
                    // Save value
                    m_cfgData.MeasuredTxPowerMinus6dBm = Helper.ConvertToTwoComplement(val);
                }
            }
            catch (Exception exception)
            {
                // Change text to red
                ((Control)sender).ForeColor = System.Drawing.Color.Red;

                // Cancel validation event
                e.Cancel = true;

                // Display error message
                ErrorMessage(exception.Message);

                // Reset to original value
                if (m_originalCfgData != null)
                {
                    ((Control)sender).ForeColor = System.Drawing.Color.Black;
                    ((Control)sender).Text = Helper.ConvertFromTwoComplement(m_originalCfgData.MeasuredTxPowerMinus6dBm).ToString();
                }
            }
        }

        private void textBoxMeasuredTxPowerMinus6_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();  
        }

        private void textBoxMeasuredTxPower0_Validating(object sender, CancelEventArgs e)
        {
            try
            {
                // Attempt to convert to integer
                int val = Convert.ToInt32(((Control)sender).Text);

                // Ensure value within bounds
                if (val < 0)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be less than {1}", labelMeasuredTxPower0.Text, 0));
                }
                else if (val > 128)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be greater than {1}", labelMeasuredTxPower0.Text, 128));
                }
                else
                {
                    // Save value
                    m_cfgData.MeasuredTxPower0dBm = Helper.ConvertToTwoComplement(val);
                }
            }
            catch (Exception exception)
            {
                // Change text to red
                ((Control)sender).ForeColor = System.Drawing.Color.Red;

                // Cancel validation event
                e.Cancel = true;

                // Display error message
                ErrorMessage(exception.Message);

                // Reset to original value
                if (m_originalCfgData != null)
                {
                    ((Control)sender).ForeColor = System.Drawing.Color.Black;
                    ((Control)sender).Text = Helper.ConvertFromTwoComplement(m_originalCfgData.MeasuredTxPower0dBm).ToString();
                }
            }
        }

        private void textBoxMeasuredTxPower0_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();  
        }

        private void textBoxMeasuredTxPower4_Validating(object sender, CancelEventArgs e)
        {
            try
            {
                // Attempt to convert to integer
                int val = Convert.ToInt32(((Control)sender).Text);

                // Ensure value within bounds
                if (val < 0)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be less than {1}", labelMeasuredTxPower4.Text, 0));
                }
                else if (val > 128)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be greater than {1}", labelMeasuredTxPower4.Text, 128));
                }
                else
                {
                    // Save value
                    m_cfgData.MeasuredTxPower4dBm = Helper.ConvertToTwoComplement(val);
                }
            }
            catch (Exception exception)
            {
                // Change text to red
                ((Control)sender).ForeColor = System.Drawing.Color.Red;

                // Cancel validation event
                e.Cancel = true;

                // Display error message
                ErrorMessage(exception.Message);

                // Reset to original value
                if (m_originalCfgData != null)
                {
                    ((Control)sender).ForeColor = System.Drawing.Color.Black;
                    ((Control)sender).Text = Helper.ConvertFromTwoComplement(m_originalCfgData.MeasuredTxPower4dBm).ToString();
                }
            }
        }

        private void textBoxMeasuredTxPower4_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();  
        }


        private void AdvertStage_Validating(object sender, CancelEventArgs e, bool isIdleTimeout, ConfigurationData.AdvSmStageSettings.Stages stage)
        {
            try
            {
                // Attempt to convert
                int val = isIdleTimeout ? (int)Convert.ToUInt32(((Control)sender).Text) : (int)Convert.ToUInt16(((Control)sender).Text);

                // Ensure value is within bounds
                int min = isIdleTimeout ? ConfigurationData.AdvSmStageSettings.MIN_ADVSM_IDLETIMEOUT_MS : ConfigurationData.AdvSmStageSettings.MIN_ADVSM_INTERVAL_MS;
                int max = isIdleTimeout ? ConfigurationData.AdvSmStageSettings.MAX_ADVSM_IDLETIMEOUT_MS : ConfigurationData.AdvSmStageSettings.MAX_ADVSM_INTERVAL_MS;
                if (((isIdleTimeout && val != 0) || !isIdleTimeout) && (val < min)) // Allow values of 0 for timeout (as disables transition to next stage)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be less than {1}", ((Control)sender).Text, min));
                }
                else if (val > max)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be greater than {1}", ((Control)sender).Text, max));
                }
                else
                {
                    // Save value
                    if (isIdleTimeout)
                    {
                        m_cfgData.AdvertisingStageSettings.SetIdleTimeoutMs(stage, Convert.ToUInt32(val));
                        ((Control)sender).Text = m_cfgData.AdvertisingStageSettings.GetIdleTimeoutMsStr(stage);

                        // If current stage's idle timeout is disbaled then disable idle timeout for subsequent stages
                        if (val == 0)
                        {
                            for (ConfigurationData.AdvSmStageSettings.Stages stageCnt = stage+1; stageCnt != ConfigurationData.AdvSmStageSettings.Stages.StageEnd; ++stageCnt)
                            {
                                m_cfgData.AdvertisingStageSettings.SetIdleTimeoutMs(stageCnt, 0);
                                switch (stageCnt)
                                {                                    
                                    case ConfigurationData.AdvSmStageSettings.Stages.Stage1:
                                        textBoxAdvertStage1Timeout.Text = m_cfgData.AdvertisingStageSettings.GetIdleTimeoutMsStr(stageCnt);
                                        break;
                                    case ConfigurationData.AdvSmStageSettings.Stages.Stage2:
                                        textBoxAdvertStage2Timeout.Text = m_cfgData.AdvertisingStageSettings.GetIdleTimeoutMsStr(stageCnt);
                                        break;
                                    case ConfigurationData.AdvSmStageSettings.Stages.Stage3:
                                        textBoxAdvertStage3Timeout.Text = m_cfgData.AdvertisingStageSettings.GetIdleTimeoutMsStr(stageCnt);
                                        break;
                                    case ConfigurationData.AdvSmStageSettings.Stages.Stage0:
                                    case ConfigurationData.AdvSmStageSettings.Stages.StageEnd:
                                    default:
                                        throw new System.Exception("Invalid argument=" + stageCnt);
                                }
                            }
                        }
                    }
                    else
                    {
                        m_cfgData.AdvertisingStageSettings.SetIntervalMs(stage, Convert.ToUInt16(val));
                        ((Control)sender).Text = m_cfgData.AdvertisingStageSettings.GetIntervalMsStr(stage);
                    }
                }
            }
            catch (Exception exception)
            {
                // Change text to red
                ((Control)sender).ForeColor = System.Drawing.Color.Red;

                // Cancel validation event
                e.Cancel = true;

                // Display error message
                ErrorMessage(exception.Message);

                // Keep Focus on this control
                ((Control)sender).Focus();

                // Reset to original value
                if (m_originalCfgData != null)
                {
                    ((Control)sender).ForeColor = System.Drawing.Color.Black;
                    if (isIdleTimeout)
                        ((Control)sender).Text = m_originalCfgData.AdvertisingStageSettings.GetIdleTimeoutMsStr(stage);
                    else
                        ((Control)sender).Text = m_originalCfgData.AdvertisingStageSettings.GetIntervalMsStr(stage);
                }
            }
        }

        private void textBoxAdvertStage0Timeout_Validating(object sender, CancelEventArgs e)
        {
            AdvertStage_Validating(sender, e, true, ConfigurationData.AdvSmStageSettings.Stages.Stage0);
        }

        private void textBoxAdvertStage0Timeout_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();   
        }

        private void textBoxAdvertStage0Interval_Validating(object sender, CancelEventArgs e)
        {
            AdvertStage_Validating(sender, e, false, ConfigurationData.AdvSmStageSettings.Stages.Stage0);
        }

        private void textBoxAdvertStage0Interval_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();   
        }

        private void textBoxAdvertStage1Timeout_Validating(object sender, CancelEventArgs e)
        {
            AdvertStage_Validating(sender, e, true, ConfigurationData.AdvSmStageSettings.Stages.Stage1);
        }

        private void textBoxAdvertStage1Timeout_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();   
        }

        private void textBoxAdvertStage1Interval_Validating(object sender, CancelEventArgs e)
        {
            AdvertStage_Validating(sender, e, false, ConfigurationData.AdvSmStageSettings.Stages.Stage1);
        }

        private void textBoxAdvertStage1Interval_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();   
        }

        private void textBoxAdvertStage2Timeout_Validating(object sender, CancelEventArgs e)
        {
            AdvertStage_Validating(sender, e, true, ConfigurationData.AdvSmStageSettings.Stages.Stage2);
        }

        private void textBoxAdvertStage2Timeout_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();   
        }

        private void textBoxAdvertStage2Interval_Validating(object sender, CancelEventArgs e)
        {
            AdvertStage_Validating(sender, e, false, ConfigurationData.AdvSmStageSettings.Stages.Stage2);
        }

        private void textBoxAdvertStage2Interval_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();   
        }

        private void textBoxAdvertStage3Timeout_Validating(object sender, CancelEventArgs e)
        {
            AdvertStage_Validating(sender, e, true, ConfigurationData.AdvSmStageSettings.Stages.Stage3);
        }

        private void textBoxAdvertStage3Timeout_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();   
        }

        private void textBoxAdvertStage3Interval_Validating(object sender, CancelEventArgs e)
        {
            AdvertStage_Validating(sender, e, false, ConfigurationData.AdvSmStageSettings.Stages.Stage3);
        }

        private void textBoxAdvertStage3Interval_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();
        }

        private void UserControlConfigurationData_Enter(object sender, EventArgs e)
        {
            // Default to device page
            tabControlMain.SelectedTab = tabPageDevice;
        }

        private void textBoxAdvertStage0IntervalBcn_Validating(object sender, CancelEventArgs e)
        {
            AdvertStage_Validating(sender, e, false, ConfigurationData.AdvSmStageSettings.Stages.Stage0);
        }

        private void textBoxAdvertStage0IntervalBcn_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();
        }

        private void textBoxAdvertStage1IntervalBcn_Validating(object sender, CancelEventArgs e)
        {
            AdvertStage_Validating(sender, e, false, ConfigurationData.AdvSmStageSettings.Stages.Stage1);
        }

        private void textBoxAdvertStage1IntervalBcn_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();
        }

        private void textBoxAdvertStageIntervalLegacy_Validating(object sender, CancelEventArgs e)
        {
            try
            {
                // Attempt to convert to decimal
                int val = Convert.ToUInt16(((Control)sender).Text);

                // Ensure value is within bounds
                if (val < ConfigurationData.MIN_ADVERT_INTERVAL_MS)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be less than {1}", ((Control)sender).Text, ConfigurationData.MIN_ADVERT_INTERVAL_MS));
                }
                else if (val > ConfigurationData.MAX_ADVERT_INTERVAL_MS)
                {
                    throw new System.Exception(string.Format("{0}:\nThe number cannot be greater than {1}", ((Control)sender).Text, ConfigurationData.MAX_ADVERT_INTERVAL_MS));
                }
                else
                {
                    // Save value
                    m_cfgData.AdvertisingIntervalMs = val;
                }
            }
            catch (Exception exception)
            {
                // Change text to red
                ((Control)sender).ForeColor = System.Drawing.Color.Red;

                // Cancel validation event
                e.Cancel = true;

                // Display error message
                ErrorMessage(exception.Message);

                // Reset to original value
                if (m_originalCfgData != null)
                {
                    ((Control)sender).ForeColor = System.Drawing.Color.Black;
                    ((Control)sender).Text = m_originalCfgData.AdvertisingIntervalMs.ToString();
                }
            }
        }

        private void textBoxAdvertStageIntervalLegacy_Validated(object sender, EventArgs e)
        {
            ((Control)sender).ForeColor = System.Drawing.Color.Black;
            EnableApplyButton();
        }
    }
}

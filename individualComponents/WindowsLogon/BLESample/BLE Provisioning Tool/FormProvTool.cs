using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Threading;
using System.Threading.Tasks;
using System.IO;
using Microsoft.Win32;
using System.Reflection;
using System.Diagnostics;
using BLE_API;

namespace BLE_Provisioning_Tool
{
    public partial class FormProvTool : Form
    {
        // Types
        enum eApplicationStatus
        {
            eApplIdle,
            eApplScan,
            eApplConnecting,
            eApplProcess
        }

        // Member variables
        private static readonly string NamedPipe = @"\\.\pipe\Mi-TokenBLEV2";
        private FormComPort m_formComPort = new FormComPort();
        private UserControlBleDeviceList m_bleDeviceList = new UserControlBleDeviceList();                
        private delegate void DelegateAddDataToPanel(BleDeviceData data);
        private static bool m_isInitialActivateEvent = true;
        private Exporter m_exporter = new Exporter();
        private BeaconData m_prevBeaconData = new BeaconData();
        private int m_prevAdvertIntervalMs = 0;
        private bool m_isSaveRecommended = false;
        static private BLE.V2_2.MiTokenBLE MiTokenBLE = null;
        private DateTime m_ts = DateTime.Now;
        private DevNameFilterType m_devNameFilterType = new DevNameFilterType();
        private Dictionary<DevNameFilterType.eType, ToolStripMenuItem> m_filterTypeDictionary = new Dictionary<DevNameFilterType.eType, ToolStripMenuItem>();
        private BLE.Core.API_RET m_serialPortStatus = BLE.Core.API_RET.BLE_API_ERR_UNKNOWN;
        private bool m_isBleScanning = false;
        private LastError m_lastError = new LastError();
        private ManualResetEventSlim m_connectionUpdateResponseEvent = new ManualResetEventSlim(false);
        private SemaphoreSlim m_connectionUpdateResponseSemaphore = new SemaphoreSlim(2);
        private ManualResetEventSlim m_procedureCompletedEvent = new ManualResetEventSlim(false);
        private ManualResetEventSlim m_connectionDisconnectedEvent = new ManualResetEventSlim(false);

        public BLE.V2_2.MiTokenBLE GetMiTokenBLE()
        {
            return MiTokenBLE;
        }

        public ManualResetEventSlim GetConnectionUpdateResponseEvent()
        {
            return m_connectionUpdateResponseEvent;
        }

        public SemaphoreSlim GetConnectionUpdateResponseSemaphore()
        {
            return m_connectionUpdateResponseSemaphore;
        }

        public ManualResetEventSlim GetProcedureCompletedEvent()
        {
            return m_procedureCompletedEvent;
        }

        public ManualResetEventSlim GetConnectionDisconnected()
        {
            return m_connectionDisconnectedEvent;
        }

        private Dictionary<eApplicationStatus, string> m_applState = new Dictionary<eApplicationStatus, string>()
        {
            { eApplicationStatus.eApplIdle, "Idle" },
            { eApplicationStatus.eApplScan, "Scanning" },
            { eApplicationStatus.eApplConnecting, "Connecting" },
            { eApplicationStatus.eApplProcess, "Processing" },
        };

        System.ServiceProcess.ServiceController getWatcherService()
        {
            try
            {
                System.ServiceProcess.ServiceController sc = new System.ServiceProcess.ServiceController("MiTokenBLEWatcher");
                return sc;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return null;
            }  
        }

        public FormProvTool()
        {
            InitializeComponent();
           
            ucConfigurationData.BackButtonClicked += new EventHandler(this.CfgBackButtonHandler);
            ucConfigurationData.ApplyButtonClicked += new EventHandler<UserControlConfigurationData.UserControlConfigurationDataEventArgs>(this.CfgApplyButtonHandler);

            // Restore from registry
            m_formComPort.RestoreFromRegistry();
            m_devNameFilterType.RestoreFromRegistry();

            // So that first time will initialize to 0
            m_prevBeaconData.Minor = 0xffff;

            // Initialize filter type dictionary
            m_filterTypeDictionary.Add(DevNameFilterType.eType.None, noneToolStripMenuItem);
            m_filterTypeDictionary.Add(DevNameFilterType.eType.MtAll, allMiTokenToolStripMenuItem);
            m_filterTypeDictionary.Add(DevNameFilterType.eType.MtBlu, miTokenBluToolStripMenuItem);
            m_filterTypeDictionary.Add(DevNameFilterType.eType.MtBcn, miTokenBeaconToolStripMenuItem);
            m_filterTypeDictionary.Add(DevNameFilterType.eType.MtBoot, miTokenBootToolStripMenuItem);

            // Ensure correct menu items are checked
            SetDevNameFilterMenuItems(m_devNameFilterType.Type);

            // Stop Watcher Service
            var sc = getWatcherService();
            try
            {
                // Only attempt to stop service if able to
                if (sc.CanStop)
                {
                    sc.Stop();
                }
            }
            catch
            {
                Debug.WriteLine(String.Format("{0}\tFormProvTool:Could not stop Watcher service", DateTime.Now));
            }

            this.DoubleBuffered = true;
        }

        private void FormProvTool_FormClosing(object sender, FormClosingEventArgs e)
        {
            // Restart watcher service
            var sc = getWatcherService();
            try
            {
                sc.Start();
            }
            catch
            {
                Debug.WriteLine(String.Format("{0}\tFormProvTool_FormClosing:Could not start Watcher service", DateTime.Now));
            }

            // Exit early if no bonds to save or not recommended to save
            if (m_exporter.IsEmpty() || !m_isSaveRecommended)
            {
                StopBleScanning();
                DisconnectFromSerial();
                return;
            }

            // Prompt as there are bonds to save
            if (DialogResult.OK == MessageBoxEx.Show("Are you sure you want to Exit?\nAll bonding information will be lost.", "Exit", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning))
            {
                StopBleScanning();
                DisconnectFromSerial();
            }
            else
            {
                e.Cancel = true;
            }
        }

        private BLE.Core.API_RET ConnectToSerial()
        {
            if (MiTokenBLE != null)
            {
                MiTokenBLE.CreateNewInstance();
            }
            else
            {
                MiTokenBLE = new BLE.V2_2.MiTokenBLE();
            }
            
            MiTokenBLE.EnableNonUniqueAttributeUUIDMode();

            // Setup command chain
            BLE.CommandChain CC = new BLE.CommandChain();
            CC.evt.attclient.procedure_completed = ProcedureCompletedEvent;
            CC.evt.connection.status = ConnectionStatusEvent;
            CC.rsp.connection.update = ConnectionUpdateResponse;
            CC.evt.attclient.attribute_value = OadService.attribute_value;
            CC.evt.connection.disconnected = ConnectionDisconnectedEvent;
            MiTokenBLE.AppendCommandChain(CC);

            // Add MiToken BLE object to dictionary
            bleDictionary.Add(MiTokenBLE, this);
#if DEBUG_BLE_API_DEBUGGER
            BLE.Core.Debug.DebugMessageCallback += GDM2;
            string errorCode;
            if (!BLE.Core.Debug.RegisterForDebugMessages(MiTokenBLE, out errorCode))
            {
                Invoke((MethodInvoker)delegate
                {
                    MessageBox.Show(string.Format("Failed to attach debugger: errCode=[{0}]", errorCode));
                });
            }
#endif
            BLE.Core.API_RET rc = MiTokenBLE.Initialize(m_formComPort.ComPort, NamedPipe, false);
            if (rc != BLE.Core.API_RET.BLE_API_SUCCESS)
            {
                if (rc == BLE.Core.API_RET.BLE_API_ERR_FAILED_TO_OPEN_COM_PORT)
                {
                    MessageBoxEx.Show("Could not attach to " + m_formComPort.ComPort + "\r\nPlease ensure this is the correct port", "Status", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
                else
                {
                    MessageBoxEx.Show("Initialize returned error code " + rc, "Status", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
            return rc;
        }

        private void DisconnectFromSerial()
        {
            if (MiTokenBLE != null)
            {

                bleDictionary.Remove(MiTokenBLE);

                try
                {
                    // Disconnect
                    MiTokenBLE.DeleteInstance();
                }
                catch
                {
                    // Unexpected Exception from DLL
                }
            }
        }

        public static Dictionary<BLE.V2_2.MiTokenBLE, FormProvTool> bleDictionary = new Dictionary<BLE.V2_2.MiTokenBLE, FormProvTool>();
#if DEBUG_BLE_API_DEBUGGER
        public static void GDM2(BLE.V2_2.MiTokenBLE instance, string message)
        {
            if (bleDictionary.ContainsKey(instance))
            {
                bleDictionary[instance].Debugger_Debugger_SendDebugMessageCallback(message);
            }
        }


        void Debugger_Debugger_SendDebugMessageCallback(string message)
        {
            if (InvokeRequired)
            {
                BeginInvoke((MethodInvoker)delegate
                {
                    Debugger_Debugger_SendDebugMessageCallback(message);
                    return;
                });

                return;
            }

            Debug.WriteLine(String.Format(message));
        }
#endif
        private bool StartBleScanning(bool isSilent = false)
        {
            // Only attempt to start scanning if com port is connected
            if (m_serialPortStatus == BLE.Core.API_RET.BLE_API_SUCCESS)
            {
                if (!m_isBleScanning)
                {
                    // Start scanner
                    if (!isSilent)
                        UpdateStatus(eApplicationStatus.eApplScan, string.Format("Scanning ({0})", m_devNameFilterType.getTypeStr()));

                    try
                    {
                        // Start polling
                        MiTokenBLE.NewPollResults += new BLE.V2_1.MiTokenBLE.NewPollResultsDelegate(MiTokenBLE_NewPollResults);
                        MiTokenBLE.AddPollThreadInstance();
                        m_isBleScanning = true;
                    }
                    catch
                    {
                        // Unexpected Exception from DLL
                    }
                }
                else
                {
                    Debug.WriteLine(String.Format("StartBleScanning:Incorrect State m_isBleScanning=[{0}]", m_isBleScanning));
                    return false;
                }
                return true;
            }
            if (!isSilent)
                MessageBoxEx.Show("Serial Port not attached!", "Status", MessageBoxButtons.OK, MessageBoxIcon.Error);
            return false;
        }

        private void StopBleScanning(bool isSilent = false)
        {
            if (!isSilent)
            {
                // Update Status
                UpdateStatus(eApplicationStatus.eApplIdle);
            }

            if (m_isBleScanning)
            {
                try
                {
                    // Stop polling                    
                    MiTokenBLE.RemovePollThreadInstance();
                    MiTokenBLE.NewPollResults -= MiTokenBLE_NewPollResults;
                    m_isBleScanning = false;
                }
                catch
                {
                    // Unexpected Exception from DLL
                }
            }
            else
            {
                Debug.WriteLine(String.Format("StopBleScanning:Incorrect State m_isBleScanning=[{0}]", m_isBleScanning));
            }
        }

        private void FormProvTool_Load(object sender, EventArgs e)
        {
            UpdateStatus(eApplicationStatus.eApplIdle);

            // Update dialog text with name and version
            Assembly assembly = Assembly.GetExecutingAssembly();
            this.Text = "Provisioning Tool V" + FileVersionInfo.GetVersionInfo(assembly.Location).FileVersion;
        }

        private void FormProvTool_Activated(object sender, EventArgs e)
        {
            // Ensure only run once
            if (!m_isInitialActivateEvent)
            {
                return;
            }
            m_isInitialActivateEvent = false;

            // Disable current form
            this.Enabled = false;

            // Display com port dialog as long
            while (m_serialPortStatus != BLE.Core.API_RET.BLE_API_SUCCESS)
            {
                if (string.IsNullOrEmpty(m_formComPort.ComPort))
                {
                    // Display as modal to ensure a com port is selected                
                    m_formComPort.ShowDialog();
                }

                // Connect to Serial
                m_serialPortStatus = ConnectToSerial();
                if (m_serialPortStatus == BLE.Core.API_RET.BLE_API_ERR_FAILED_TO_OPEN_COM_PORT)
                {
                    // Exit early
                    break;
                }
            }

            if (m_serialPortStatus == BLE.Core.API_RET.BLE_API_SUCCESS)
            {
                // Save COM Port to registry only if sucessfully opened
                m_formComPort.SaveToRegistry();

                // Start scanning
                StartBleScanning();
            }
            
            // Re-enable current form
            this.Enabled = true;            
        }

        public class UISession : IDisposable
        {
            private FormProvTool m_formProvTool = null;
            private bool m_isAutoScanStart = true;

            public UISession(FormProvTool formProvTool, bool isAutoScanStart = true)
            {
                m_formProvTool = formProvTool;
                m_isAutoScanStart = isAutoScanStart;                
                m_formProvTool.PrepareForConnection();
            }

            public void ForceClose()
            {
                // Irregardless of auto scan start flag
                m_formProvTool.PrepareForScanning();
            }

            public void Dispose()
            {
                if (m_isAutoScanStart)
                {
                    // Restart Scanning
                    m_formProvTool.PrepareForScanning();
                }
            }
        }

        public void PrepareForConnection()
        {
            // Stop scanning
            StopBleScanning();

            // Disable Menu Stip
            SetMenuStipEnabled(false);

            // Disable device selection tab
            WizardPages.EnableTab(tabPageDeviceSelection, false);
        }

        public void PrepareForScanning()
        {
            // Enable device selection tab
            WizardPages.EnableTab(tabPageDeviceSelection, true);

            // Enable Menu Stip
            SetMenuStipEnabled(true);

            // Start scanning
            StartBleScanning();
        }

        private async void ProvisionButtonHandler(object sender, UserControlBleDevice.UserControlBleDeviceEventArgs e)
        {
            using (var uiSession = new UISession(this))
            {
                if ((e.Data.UseCase == ConfigurationData.eUseCase.eBeacon) || (e.Data.UseCase == ConfigurationData.eUseCase.eAuthBeacon))
                {
                    // Automatically creating a bond and set configuration data over a secure session
                    await PerformAutoProvision(e.Data);
                }
                else if (e.Data.UseCase == ConfigurationData.eUseCase.eBlu)
                {
                    // Create bond with device
                    await PerformCreateBond(e.Data);
                }
                else
                {
                    MessageBox.Show("Invalid Use Case", "Error",MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        }

        private async void UnpairButtonHandler(object sender, UserControlBleDevice.UserControlBleDeviceEventArgs e)
        {
            using (var uiSession = new UISession(this))
            {
                // Factory reset with device
                await PerformFactoryReset(e.Data);
            }
        }

        private async void PingButtonHandler(object sender, UserControlBleDevice.UserControlBleDeviceEventArgs e)
        {
            using (var uiSession = new UISession(this))
            {
                // Request an audible ping from device
                await PerformPing(e.Data);
            }
        }

        private async void ConfigureButtonHandler(object sender, UserControlBleDevice.UserControlBleDeviceEventArgs e)
        {
            using (var uiSession = new UISession(this, false))
            {
                // Retrieve configuration data from device
                if (await PerformRetrieveCfgData(e.Data))
                {
                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplIdle);

                    // Enable Menu Stip
                    SetMenuStipEnabled(true);

                    // Switch to Configuration Selection
                    wizardPages.SelectTab(tabPageCfgSelection);

                    // Purposely do not re-start scanning
                }
                else
                {
                    // Manually close session
                    uiSession.ForceClose();
                }
            }
        }

        private async void UpdateButtonHandler(object sender, UserControlBleDevice.UserControlBleDeviceEventArgs e)
        {
            using (var uiSession = new UISession(this))
            {
                if (e.Data.IsOadSvcExists && e.Data.IsOadFirmware)
                {
                    DialogResult result = MessageBox.Show("Select 'Yes' to update the target image.\nSelect 'No' to upgrade (invalidate) the current the image.", "Update", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Question);
                    if (result == DialogResult.Yes)
                    {
                        // Over the Air Download of an image
                        await PerformOAD(e.Data);
                    }
                    else if (result == DialogResult.No)
                    {
                        // Invalidate device's current image
                        await PerformInvalidate(e.Data);
                    }
                }
                else if (e.Data.IsOadSvcExists && !e.Data.IsOadFirmware)
                {
                    // Over the Air Download of an image
                    await PerformOAD(e.Data);
                }
                else if (!e.Data.IsOadSvcExists && e.Data.IsOadFirmware)
                {
                    // Invalidate device's current image
                    await PerformInvalidate(e.Data);
                }
            }
        }

        private async void RefreshButtonHandler(object sender, UserControlBleDevice.UserControlBleDeviceEventArgs e)
        {
            using (var uiSession = new UISession(this))
            {
                // Clear current device
                clearDevicePage(e.Data);
            }
        }

        private void CfgBackButtonHandler(object sender, EventArgs e)
        {
            // Clear configuration settings
            ucConfigurationData.Reset();

            // Switch to Device Selection
            wizardPages.SelectTab(tabPageDeviceSelection);

            // Re-start scanning
            StartBleScanning();
        }

        private async void CfgApplyButtonHandler(object sender, UserControlConfigurationData.UserControlConfigurationDataEventArgs e)
        {
            // Switch to device selection tab which which will enable th device tab
            wizardPages.SelectTab(tabPageDeviceSelection);

            using (var uiSession = new UISession(this))
            {
                // Apply Configuration Settings to device
                await PerformApplyCfgData(e.OldData, e.NewData);
            }
        }

        async void MiTokenBLE_NewPollResults(BLE.V2_1.MiTokenBLE sender)
        {
            if (!m_isBleScanning)
            {
                return;
            }

            // Cast to V2.2 object
            sender = (BLE.V2_2.MiTokenBLE)sender;

            // Throttle processing advertising data to prevent UI lagging
            TimeSpan ts = DateTime.Now.Subtract(m_ts);
            if (ts.TotalMilliseconds < 500.0)
            {
                return;
            }
            m_ts = DateTime.Now;

            // Cache BLE data to ensure only only invoke is called once
            List<BleDeviceData> cachedData = new List<BleDeviceData>();

            // Process advertising data            
            int devFoundCount = sender.GetDeviceFoundCount();
            for (int i = 0; i < devFoundCount; ++i)
            {
                // Retrieve Device Name
                string devName = "";
                byte[] nameArray;
                if (MiTokenBLE.GetDeviceMetaInfo(i, GapAdvertDataTypes.GAP_ADTYPE_LOCAL_NAME_COMPLETE, out nameArray) == BLE.Core.API_RET.BLE_API_SUCCESS)
                {
                    devName = System.Text.Encoding.UTF8.GetString(nameArray);
                }

                // Retrieve iBeacon data
                byte[] beaconArray;
                BeaconData beaconData = new BeaconData();
                if (MiTokenBLE.GetDeviceMetaInfo(i, GapAdvertDataTypes.GAP_ADTYPE_MANUFACTURER_SPECIFIC, out beaconArray) == BLE.Core.API_RET.BLE_API_SUCCESS)
                {
                    // Attempt to initialise with iBeacon data
                    beaconData.Init(ref beaconArray);
                }

                // Ensure device name and iBeacon data is not empty
                bool isClearedForProcessing = false;
                switch (m_devNameFilterType.Type)
                {
                    case DevNameFilterType.eType.None:
                        isClearedForProcessing = true;
                        break;

                    case DevNameFilterType.eType.MtAll:
                        if (devName.StartsWith(m_devNameFilterType.getTypeStr()))
                        {
                            isClearedForProcessing = true;
                        }
                        break;

                    case DevNameFilterType.eType.MtBlu:
                    case DevNameFilterType.eType.MtBcn:
                    case DevNameFilterType.eType.MtBoot:
                        if (devName.CompareTo(m_devNameFilterType.getTypeStr()) == 0)
                        {
                            isClearedForProcessing = true;
                        }
                        break;
                }

                if (isClearedForProcessing)
                {
                    // Get Device Info                    
                    BLE.Core.Structures.DeviceInfo devInfo = new BLE.Core.Structures.DeviceInfo();
                    MiTokenBLE.GetDeviceInfo(i, ref devInfo);

                    bool isBonded = m_exporter.IsExisit(new BondInfo(new DeviceAddress(devInfo.macAddress)));

                    // Create Device data and cache
                    BleDeviceData bleDevData = new BleDeviceData(devInfo.macAddress, devName, beaconData, (byte)devInfo.RSSI, (UInt32)devInfo.lastPollSeen, isBonded);
                    cachedData.Add(bleDevData);
                }
            }

            // Sort devices based on device name
            List<BleDeviceData> sortedList = cachedData.OrderBy(o => o.DeviceName).ToList();            

            // Update UI with BLE data
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)async delegate
                {
                    await UpdateUiAllDevice(sortedList);
                });
            }
            else
            {
                await UpdateUiAllDevice(sortedList);
            }
        }

        private async Task UpdateUiAllDevice(List<BleDeviceData> devices)
        {
            // Note: Important! Temporarily Stop polling            
            StopBleScanning(true); // Silence

            // Attempt to add/update each device
            foreach (BleDeviceData tempData in devices)
            {
                // Attempt to add device to panel
                UserControlBleDevice ucBleDevice;
                if (m_bleDeviceList.Add(ref flowLayoutPanelBleDevices, tempData, out ucBleDevice))
                {
                    // New device so add event handlers
                    ucBleDevice.ProvisionButtonClicked += new EventHandler<UserControlBleDevice.UserControlBleDeviceEventArgs>(this.ProvisionButtonHandler);
                    ucBleDevice.UnpairButtonClicked += new EventHandler<UserControlBleDevice.UserControlBleDeviceEventArgs>(this.UnpairButtonHandler);
                    ucBleDevice.PingButtonClicked += new EventHandler<UserControlBleDevice.UserControlBleDeviceEventArgs>(this.PingButtonHandler);
                    ucBleDevice.ConfigureButtonClicked += new EventHandler<UserControlBleDevice.UserControlBleDeviceEventArgs>(this.ConfigureButtonHandler);
                    ucBleDevice.UpdateButtonClicked += new EventHandler<UserControlBleDevice.UserControlBleDeviceEventArgs>(this.UpdateButtonHandler);
                    ucBleDevice.RefreshButtonClicked += new EventHandler<UserControlBleDevice.UserControlBleDeviceEventArgs>(this.RefreshButtonHandler);

                    // Initialise new device for first time and finialize if complete
                    await InitializeDeviceControls(tempData, ucBleDevice);
                }
                else
                {
#if false // Do not do this as can end up in an infinite loop if device controls cannot be finialised
                    // Check if form is finialized
                    if (!ucBleDevice.IsFormFinialized())
                    {
                        // Attempt to initiaze device again
                        await InitializeDeviceControls(tempData, ucBleDevice);
                    }
#endif
                }
            }

            // Note: Important! Start polling
            StartBleScanning(true);  // Silence
        }

        private async Task InitializeDeviceControls(BleDeviceData bleDeviceData, UserControlBleDevice ucBleDevice)
        {
#if false
            // Prevents connecting to device if it isn't flagged as a mi-token device
            if (!ucBleDevice.DeviceName.StartsWith(DevNameFilterType.getTypeStr(DevNameFilterType.eType.MtAll)))
            {
                ucBleDevice.SetLabelButtonProcessingText("No Supported Services");
                return;
            }
#endif

            // Disable entire form to prevent user commands
            this.Enabled = false;

#if DEBUG_DIAGNOSTICS
            Stopwatch sw = new Stopwatch();
            sw.Start();
#endif
            // Attempt to connect to device
            Debug.WriteLine(String.Format("{0}\tPopulateButtons:Create Connection Address=[{1:X}]", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt"), bleDeviceData.Address.GetRawString()));
            using (var session = new ConnectedSession(MiTokenBLE, bleDeviceData.Address, m_procedureCompletedEvent, m_connectionDisconnectedEvent, 900))
            {
                try
                {
                    // Check device is connected
                    if (!await session.Open()) throw new Exception("");

                    // Update Connection Paramteres
                    if (!await BLE_AdvFunctions.UpdateConnectionParameters(session.Device, m_connectionUpdateResponseEvent, m_connectionUpdateResponseSemaphore)) throw new Exception("");

                    // Check services exists
                    ucBleDevice.IsDevInfoSvcExists = await BLE_AdvFunctions.FindServiceAttributes(session.Device, DevInfoService.SERV_UUID);
                    ucBleDevice.IsSecureBondSvcExists = await BLE_AdvFunctions.FindServiceAttributes(session.Device, SecureBondService.SERV_UUID);
                    ucBleDevice.IsAuthenSvcExists = await BLE_AdvFunctions.FindServiceAttributes(session.Device, AuthenService.SERV_UUID);
                    ucBleDevice.IsOadSvcExists = await BLE_AdvFunctions.FindServiceAttributes(session.Device, OadService.SERV_UUID, true);
                    ucBleDevice.IsSysCfgSvcExists = await BLE_AdvFunctions.FindServiceAttributes(session.Device, SysCfgService.SERV_UUID);

                    // Exit early and prevent finialization if no connection
                    if (session.Device.IsConnected() == BLE.Core.CONN_RET.BLE_CONN_ERR_NOT_CONNECTED) throw new Exception();

                    // Check Buzzer in order of device information then system configuration service
                    ucBleDevice.IsBuzzerExists = false;
                    String fwRevStr = "";
                    ConfigurationData.Revision_t fwRev = new ConfigurationData.Revision_t();
                    if (ucBleDevice.IsDevInfoSvcExists)
                    {
                        // Get firmware revision                    
                        {
                            var result = await BLE_AdvFunctions.ReadRequest(session.Device, DevInfoService.ATT_FWREV_UUID, BLE_AdvFunctions.UNDEFINED_LENGTH);
                            if (result.Item1)
                            {
                                fwRevStr = System.Text.Encoding.UTF8.GetString(result.Item2);
                            }
                        }
                        fwRev = ConfigurationData.GetFirmwareRevision(fwRevStr);

                        // Retrieve use case
                        ucBleDevice.UseCase = ConfigurationData.GetUseCase(fwRevStr);

                        // Check if form factor has a buzzer
                        ConfigurationData.eFormFactor ff = await DevInfoService.ReadHardwareRevision(session.Device);
                        if (ff == ConfigurationData.eFormFactor.eVTAG || ff == ConfigurationData.eFormFactor.eF1)
                        {
                            ucBleDevice.IsBuzzerExists = true;
                        }
                    }

                    // Check if buzzer has not already been enabled via device information service
                    if (ucBleDevice.IsSysCfgSvcExists && !ucBleDevice.IsBuzzerExists)
                    {
#if DEBUG_DIAGNOSTICS
                        Stopwatch sw2 = new Stopwatch();
                        sw2.Start();
#endif
                        var result = await SysCfgService.ReadPeripherals(session.Device);
                        if (result.Item1)
                        {
                            if (result.Item2.IsExists(ConfigurationData.Peripherals.PeriID.BUZZER))
                            {
                                ucBleDevice.IsBuzzerExists = true;
                            }
                        }
#if DEBUG_DIAGNOSTICS
                        sw2.Stop();
                        Debug.WriteLine(String.Format("{0}\tPopulateButtons:buzzerCheck=[{1}]", DateTime.Now, sw2.Elapsed));
#endif
                    }

                    // Retrieve if provisioned for beacon/auth-beacon uses cases
                    ucBleDevice.IsProvisioned = false;
                    if (ucBleDevice.IsSysCfgSvcExists && ((ucBleDevice.UseCase == ConfigurationData.eUseCase.eBeacon) || (ucBleDevice.UseCase == ConfigurationData.eUseCase.eAuthBeacon)))
                    {
                        ConfigurationData cfgData = new ConfigurationData();
                        await SysCfgService.ReadAttRequest(session.Device, SysCfgService.CmdId.GetIsProvisioned, cfgData);
                        ucBleDevice.IsProvisioned = cfgData.IsProvisioned;
                    }

                    // Retrieve if OAD firmware
                    ucBleDevice.IsOadFirmware = false;
                    if (ucBleDevice.IsDevInfoSvcExists)
                    {
                        ucBleDevice.IsOadFirmware = ConfigurationData.isOadFirmware(fwRevStr);
                    }

                    // Retrieve if can create bond
                    ucBleDevice.IsAlreadyBonded = false;
                    if (ucBleDevice.IsSecureBondSvcExists)
                    {

                        if (fwRev.Major >= 1 && fwRev.Minor >= 11)
                        {
                            // Create data to send i.e. CMD + DEVICE_ID                        
                            byte[] cmdData = Helper.BuildByte(new byte[][] {
                                new byte[] { (byte)SecureBondService.CmdId.CHECK_BOND },
                                Helper.CreateDeviceID(bleDeviceData.Address)
                            });

                            // Send command data
                            await BLE_AdvFunctions.WriteRequest(session.Device, SecureBondService.ATT_CFG_UUID, cmdData);

                            // Determine if can create a bond for this device; if bond already exists then SUCCESS otherwise error
                            ucBleDevice.IsAlreadyBonded = (m_lastError.Value != (int)AppErrorCodes.ATT_ERR_SECUREBOND_CMD_CHECK_BOND_FAIL);
                        }
                        else
                        { // Kept for backwards compatibiltiy with firmware pre V1.11
                            // Create data to send i.e. CMD + DEVICE_ID                        
                            byte[] cmdData = Helper.BuildByte(new byte[][] {
                                new byte[] { (byte)SecureBondService.CmdId.ADD_BOND },
                                Helper.CreateDeviceID(bleDeviceData.Address)
                            });

                            // Send command data
                            await BLE_AdvFunctions.WriteRequest(session.Device, SecureBondService.ATT_CFG_UUID, cmdData);

                            // Determine if can create a bond for this device; if bond already exists or maximum amount of bonds reached then error otherwise SUCCESS (reverse logic to checking bond)
                            ucBleDevice.IsAlreadyBonded = (m_lastError.Value == (int)AppErrorCodes.ATT_ERR_SECUREBOND_CFG_ADD_FAIL);
                        }

                        // Since device disconnects after no need to check for anything else
                    }

                    // Enable controls accordingly to service availability
                    ucBleDevice.RefreshServiceControls();
                }
                catch (Exception e)
                {
                    // Force an early close
                    session.Close();

                    // Update UI
                    ucBleDevice.SetLabelButtonProcessingText("");
                    ucBleDevice.EnableButton(UserControlBleDevice.ButtonType.eRefreshButton, true);
                }
                finally
                {
#if DEBUG_DIAGNOSTICS
                    sw.Stop();
                    Debug.WriteLine(String.Format("{0}\tPopulateButtons:Elapsed=[{1}]", DateTime.Now, sw.Elapsed));
#endif
                    // Re-enable entire form to allow user commands
                    this.Enabled = true;
                }
            }
        }
        
        private void UpdateStatus(eApplicationStatus status, String customText = "", bool isCustomProgressBarValue = false)
        {
            try
            {
                if (customText == "")
                    this.toolStripStatusLabel.Text = this.m_applState[status];
                else
                    this.toolStripStatusLabel.Text = customText;

                if (!isCustomProgressBarValue)
                {
                    switch (status)
                    {
                        case eApplicationStatus.eApplScan:
                            this.toolStripProgressBar.Style = ProgressBarStyle.Marquee;
                            this.toolStripProgressBar.MarqueeAnimationSpeed = 60;
                            break;

                        case eApplicationStatus.eApplConnecting:
                        case eApplicationStatus.eApplProcess:
                            this.toolStripProgressBar.Style = ProgressBarStyle.Marquee;
                            this.toolStripProgressBar.MarqueeAnimationSpeed = 60;
                            break;

                        case eApplicationStatus.eApplIdle:
                            this.toolStripProgressBar.Value = 0;
                            this.toolStripProgressBar.Style = ProgressBarStyle.Continuous;
                            this.toolStripProgressBar.MarqueeAnimationSpeed = 0;
                            break;
                    }
                }
                else
                {
                    this.toolStripProgressBar.Style = ProgressBarStyle.Blocks;
                }
            }
            catch (KeyNotFoundException)
            {
                Debug.WriteLine(String.Format("ApplicationStatus[" + status + "] is not found"));
            }
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        public class ConnectedSession : IDisposable
        {
            public BLE.V2_2.Device Device { get; private set; }
            private bool isConnected = false;
            public ManualResetEventSlim ProcedureCompletedEvent = null;
            public ManualResetEventSlim ConnectionDisconnectedEvent = null;
            int timeoutMs = -1;
            
            public ConnectedSession(BLE.V2_2.MiTokenBLE ble, DeviceAddress devAddress, ManualResetEventSlim pcEvent, ManualResetEventSlim cdEvent, int timeoutMs = -1)
            {                
                Device = new BLE.V2_2.Device(ble, devAddress.Value);
                ProcedureCompletedEvent = pcEvent;
                ConnectionDisconnectedEvent = cdEvent;

                // Ensure event is reset before session is opened
                ConnectionDisconnectedEvent.Reset();
            }

            public async Task<bool> Open()
            {
                // Check device is connected                
                isConnected = await BLE_AdvFunctions.CheckConnection(Device, ProcedureCompletedEvent, (timeoutMs == -1) ? BLE_AdvFunctions.PROCESSING_TIMEOUT_CONNECTION_MS : timeoutMs);
                return isConnected;
            }

            public void Close()
            {
                // Disconnect from device
                if (isConnected)
                {
                    Device.Disconnect();
                }
            }

            public void Dispose()
            {
                Close();
            }
        };

        private async Task<bool> PerformPing(BleDeviceData bleDeviceData)
        {
            // Update status bar
            UpdateStatus(eApplicationStatus.eApplConnecting);

            // Start Session
            using (var session = new ConnectedSession(MiTokenBLE, bleDeviceData.Address, m_procedureCompletedEvent, m_connectionDisconnectedEvent))
            {
                try
                {
                    // Check device is connected
                    if (!await session.Open()) throw new Exception("");

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplProcess, "Sending audible ping");

                    // Update Connection Paramteres
                    if (!await BLE_AdvFunctions.UpdateConnectionParameters(session.Device, m_connectionUpdateResponseEvent, m_connectionUpdateResponseSemaphore)) throw new Exception("");

                    // Ensure Secure Bond Service Exists
                    if (!await BLE_AdvFunctions.FindServiceAttributes(session.Device, SecureBondService.SERV_UUID)) throw new Exception("");

                    // Attempt to send Ping V2 command
                    SecureBondService.PingBurstCfg pingBurstCfg = new SecureBondService.PingBurstCfg();
                    pingBurstCfg.DurationMs = 100;
                    pingBurstCfg.IntervalMs = 150;
                    pingBurstCfg.AddPing(4096); // High tone
                    pingBurstCfg.AddPing(3072); // Mid tone
                    pingBurstCfg.AddPing(2048); // Low tone
                    bool result = await SecureBondService.InitiatePingV2(session.Device, pingBurstCfg);

                    // If Ping V2 command fails then use Ping V1; assuming arguments are valid
                    if (m_lastError.Value != 0 || !result)
                    {
                        // Ping device
                        if (!await SecureBondService.InitiatePing(session.Device)) throw new Exception("");
                    }
                }
                catch (Exception e)
                {
                    // Force an early close
                    session.Close();

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplIdle);

                    // Display error message
                    MessageBoxEx.Show((e.Message.Equals("") ? "Processing Timeout" : e.Message), "Status", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return false;
                }
            }
            return true;
        }

        private async Task<bool> PerformRetrieveCfgData(BleDeviceData bleDeviceData)
        {
            // Update status bar
            UpdateStatus(eApplicationStatus.eApplConnecting);

            // Flag special case in which only Device Information Service attributes are retrieved and populated
            bool isUseCaseBoot = (bleDeviceData.UseCase == ConfigurationData.eUseCase.eBoot);

            // Start Session
            using (var session = new ConnectedSession(MiTokenBLE, bleDeviceData.Address, m_procedureCompletedEvent, m_connectionDisconnectedEvent))
            {
                try
                {
                    // Check device is connected
                    if (!await session.Open()) throw new Exception("");

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplProcess, "Retrieving configuration");

                    // Update Connection Paramteres
                    if (!await BLE_AdvFunctions.UpdateConnectionParameters(session.Device, m_connectionUpdateResponseEvent, m_connectionUpdateResponseSemaphore)) throw new Exception("");

                    // Ensure Device Information Service Exists
                    if (!await BLE_AdvFunctions.FindServiceAttributes(session.Device, DevInfoService.SERV_UUID)) throw new Exception("");

                    // Ensure System Configuration Service Exists
                    if (!await BLE_AdvFunctions.FindServiceAttributes(session.Device, SysCfgService.SERV_UUID))
                    {
                        if (!isUseCaseBoot)
                        {
                            throw new Exception("");
                        }
                    } 

                    // Update with device address
                    ConfigurationData cfgData = new ConfigurationData();
                    cfgData.Address = bleDeviceData.Address;
                    if (isUseCaseBoot)
                    {
                        cfgData.DeviceName = bleDeviceData.DeviceName;
                    }

                    // Retrieve Configuration Data
                    if (!await DevInfoService.ReadAllAtt(session.Device, cfgData)) throw new Exception("");
                    if (!await SysCfgService.ReadAllAtt(session.Device, cfgData))
                    {
                        if (!isUseCaseBoot)
                        {
                            throw new Exception("");
                        }
                    }
#if false
                    // Populate Configuration tab from advertising data
                    cfgData.DeviceUUID = bleDeviceData.UUID;
                    cfgData.DeviceName = bleDeviceData.DeviceName;
                    cfgData.Major = bleDeviceData.Major;
                    cfgData.Minor = bleDeviceData.Minor;
#endif

                    // Indicate whether current device has a bond
                    cfgData.IsBonded = m_exporter.IsExisit(new BondInfo(bleDeviceData.Address));

                    // Populate Configuration tab with data from device
                    ucConfigurationData.Init(cfgData, isUseCaseBoot);
                }
                catch (Exception e)
                {
                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplIdle);

                    // Display error message
                    MessageBoxEx.Show((e.Message.Equals("") ? "Processing Timeout" : e.Message), "Status", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return false;
                }         
            }
            return true;
        }


        private async Task<bool> PerformInvalidate(BleDeviceData bleDeviceData)
        {
            // Confimation prompt
            if (DialogResult.OK != MessageBoxEx.Show("This operation will invalidate this current image, which is irreversable.\nAre you sure you want to continue?", "Confirm", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning))
            {
                return false;
            }

            // Update status bar
            UpdateStatus(eApplicationStatus.eApplConnecting);

            // Start Session
            ConfigurationData cfgData = new ConfigurationData();
            bool isUseInvalidateEx = false;
            using (var session = new ConnectedSession(MiTokenBLE, bleDeviceData.Address, m_procedureCompletedEvent, m_connectionDisconnectedEvent))
            {
                try
                {
                    // Check device is connected
                    if (!await session.Open()) throw new Exception("");

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplProcess, "Invalidating current image");

                    // Update Connection Paramteres
                    if (!await BLE_AdvFunctions.UpdateConnectionParameters(session.Device, m_connectionUpdateResponseEvent, m_connectionUpdateResponseSemaphore)) throw new Exception("");

                    // Ensure System Configuration Service Exists
                    if (!await BLE_AdvFunctions.FindServiceAttributes(session.Device, SysCfgService.SERV_UUID)) throw new Exception("");

                    // Ensure Device Information Service Exists
                    if (!await BLE_AdvFunctions.FindServiceAttributes(session.Device, DevInfoService.SERV_UUID)) throw new Exception("");

                    // Retrieve Current Firmware Revision                    
                    if (!await DevInfoService.ReadAllAtt(session.Device, cfgData)) throw new Exception("");

                    // Determine whether to use old or new invalidate functionality by retrieving Miniumum Firmware Revision
                    string minFwRev = String.Empty;
                    var result = await SysCfgService.ReadMinimumFirmwareRevision(session.Device); 
                    if (result.Item1)
                    {
                        minFwRev = result.Item2;

                        // Check current revision is at least minimum revision in order to use new Invalidate function
                        ConfigurationData.Revision_t minRev = ConfigurationData.GetFirmwareRevision(minFwRev);
                        ConfigurationData.Revision_t currRev = cfgData.GetFirmwareRevision();
                        if ((currRev.Major >= 1) && (currRev.Minor >= 7))
                        {
                            isUseInvalidateEx = true;

                        }
                        else
                        {
                            isUseInvalidateEx = false;
                        }
                    }
                    else
                    {
                        isUseInvalidateEx = false;
                    }

                    if (isUseInvalidateEx)
                    {
                        // Use new invalidate function
                        if (!await SysCfgService.InvalidateDeviceEx(session.Device)) throw new Exception("");
                    }
                    else
                    {
                        // Attribute does not exist or does not meet minimum firmware revision so use V1 invalidation
                        if (!await SysCfgService.InvalidateDevice(session.Device)) throw new Exception("");
                    }
                    
                }
                catch (Exception e)
                {
                    // Force an early close
                    session.Close();

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplIdle);

                    // Display error message
                    MessageBoxEx.Show((e.Message.Equals("") ? "Processing Timeout" : e.Message), "Status", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return false;
                }
            }

            // Post disconnect processing...

            // Clear device page
            clearDevicePage(bleDeviceData);

            // Update status bar
            UpdateStatus(eApplicationStatus.eApplIdle);

            // Consider form factor with buttons
            bool isDeviceHasPushButtons = false;
            ConfigurationData.eFormFactor formFactor = cfgData.GetFormFactor();
            switch (formFactor)
            {
                case ConfigurationData.eFormFactor.eVTAG:
                case ConfigurationData.eFormFactor.eF1:
                    isDeviceHasPushButtons = true;
                    break;
            }

            // Prompt user appropriately
            if (isUseInvalidateEx && isDeviceHasPushButtons)
            {
                MessageBoxEx.Show("Press and hold button(s) on device to complete invalidation", "Status", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            else
            {
                MessageBoxEx.Show("Invalidation successful", "Status", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            return true;
        }

        private async Task<bool> PerformApplyCfgData(ConfigurationData oldCfgData, ConfigurationData newCfgData)
        {
            // Update status bar
            UpdateStatus(eApplicationStatus.eApplConnecting);

            // Start Session
            // Take address from either configuration (should not have changed)
            BondInfo bondInfo;
            using (var session = new ConnectedSession(MiTokenBLE, oldCfgData.Address, m_procedureCompletedEvent, m_connectionDisconnectedEvent))
            {
                try
                {
                    // Check device is connected
                    if (!await session.Open()) throw new Exception("");

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplProcess, "Finding bond");

                    // Attempt to find bonding information; Take address from either configuration (should not have changed)                    
                    if (!m_exporter.GetBond(new BondInfo(oldCfgData.Address), out bondInfo)) throw new Exception("No bonding information found");

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplProcess, "Creating secure session", true);

                    // Use bond's shared secret to secure a session
                    IProgress<int> progressBarStatus = new Progress<int>(value => { toolStripProgressBar.Value = value; });
                    SessionData sessionData = await AuthenService.EstablishSecureSession(session.Device, m_connectionUpdateResponseEvent, m_connectionUpdateResponseSemaphore, bondInfo.DeviceData.Address, bondInfo.BondSharedSecret, progressBarStatus, session.ProcedureCompletedEvent);
                    if (!sessionData.IsAuthenticated) throw new Exception("Could not establish a session");                    

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplProcess, "Applying changes");

                    // Ensure System Configuration Service Exists
                    if (!await BLE_AdvFunctions.FindServiceAttributes(session.Device, SysCfgService.SERV_UUID)) throw new Exception("");

                    // Update affected configuration
                    if (!await SysCfgService.UpdateConfiguration(session.Device, oldCfgData, newCfgData, sessionData)) throw new Exception("");

                    // Only finialise provisioning if necessary
                    ConfigurationData.eUseCase useCase = newCfgData.GetUseCase();
                    if ((useCase == ConfigurationData.eUseCase.eBeacon) || (useCase == ConfigurationData.eUseCase.eAuthBeacon))
                    {
                        // Finialize provisioning if requested
                        if (newCfgData.IsProvisioned && !oldCfgData.IsProvisioned)
                        {
                            // Finialize provision
                            if (!await SysCfgService.FinializeProvision(session.Device, sessionData)) throw new Exception("");
                        }
                    }
                }
                catch(Exception e)
                {
                    MessageBoxEx.Show((e.Message.Equals("") ? "Processing Timeout" : e.Message), "Status", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return false;
                }
            }

            // Post disconnect processing...

            // Update export information
            m_exporter.Update(bondInfo, newCfgData._BeaconData, newCfgData);

            // Since bonds have changed, set flag to recommened bonds to be saved
            m_isSaveRecommended = true;

            // Clear device page
            clearDevicePage(new BleDeviceData(oldCfgData.Address));

            // Update status bar
            UpdateStatus(eApplicationStatus.eApplIdle);

            // Display result message
            MessageBoxEx.Show("Changes Saved", "Status", MessageBoxButtons.OK, MessageBoxIcon.Information);
            return true;
        }

        private async Task<bool> PerformRemoveBond(BleDeviceData bleDeviceData)
        {
            // Update status bar
            UpdateStatus(eApplicationStatus.eApplConnecting);

            // Start Session
            using (var session = new ConnectedSession(MiTokenBLE, bleDeviceData.Address, m_procedureCompletedEvent, m_connectionDisconnectedEvent))
            {
                try
                {
                    // Check device is connected
                    if (!await session.Open()) throw new Exception("");

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplProcess, "Removing bond");

                    // Update Connection Paramteres
                    if (!await BLE_AdvFunctions.UpdateConnectionParameters(session.Device, m_connectionUpdateResponseEvent, m_connectionUpdateResponseSemaphore)) throw new Exception("");

                    //*** Establish a secure connection via authentication service                
                    // Attempt to find bonding information                
                    BondInfo bondInfo;
                    if (!m_exporter.GetBond(new BondInfo(bleDeviceData.Address), out bondInfo)) throw new Exception("No bonding information found");

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplProcess, "Creating secure session", true);

                    // Use bond's shared secret to secure a session
                    IProgress<int> progressBarStatus = new Progress<int>(value => { toolStripProgressBar.Value = value; });
                    SessionData sessionData = await AuthenService.EstablishSecureSession(session.Device, m_connectionUpdateResponseEvent, m_connectionUpdateResponseSemaphore, bondInfo.DeviceData.Address, bondInfo.BondSharedSecret, progressBarStatus, session.ProcedureCompletedEvent);
                    if (!sessionData.IsAuthenticated) throw new Exception("Could not establish a session");

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplProcess, "Removing bond");

                    // Ensure Secure Bond Service Exists
                    if (!await BLE_AdvFunctions.FindServiceAttributes(session.Device, SecureBondService.SERV_UUID)) throw new Exception("");

                    // Send command to remove bond               
                    if (!await SecureBondService.RemoveBond(session.Device, sessionData, bondInfo)) throw new Exception("");
                    //TODO need to handle when removal fails

                    // Enable provision button
                    m_bleDeviceList.UpdateProvision(bleDeviceData, true);

                    // Update exporter bond information
                    m_exporter.RemoveBond(new BondInfo(bleDeviceData.Address));

                    // Since bonds have changed, set flag to recommened bonds to be saved
                    m_isSaveRecommended = true;
                }
                catch (Exception e)
                {
                    // Force an early close
                    session.Close();

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplIdle);

                    // Display error message
                    MessageBoxEx.Show((e.Message.Equals("") ? "Processing Timeout" : e.Message), "Status", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return false;
                }
            }

            // Post disconnect processing...

            // Clear device page
            clearDevicePage(bleDeviceData);

            // Update status bar
            UpdateStatus(eApplicationStatus.eApplIdle);

            // Display result message
            MessageBoxEx.Show("Bond removed successfully!", "Status", MessageBoxButtons.OK, MessageBoxIcon.Information);
            return true;
        }

        private async Task<bool> PerformFactoryReset(BleDeviceData bleDeviceData)
        {
            // Confirmation prompt
            if (DialogResult.OK != MessageBoxEx.Show("Resetting this device will remove all bonds and (if required) reset provisioning. Are you sure you want to continue?", "Confirm", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning))
            {
                return false;
            }

            // Update status bar
            UpdateStatus(eApplicationStatus.eApplConnecting);

            // Start Session
            using (var session = new ConnectedSession(MiTokenBLE, bleDeviceData.Address, m_procedureCompletedEvent, m_connectionDisconnectedEvent))
            {
                try
                {
                    // Check device is connected
                    if (!await session.Open()) throw new Exception("");

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplProcess, "Resetting device");

                    // Update Connection Paramteres
                    if (!await BLE_AdvFunctions.UpdateConnectionParameters(session.Device, m_connectionUpdateResponseEvent, m_connectionUpdateResponseSemaphore)) throw new Exception("");

                    // Ensure Secure Bond Service Exists
                    if (!await BLE_AdvFunctions.FindServiceAttributes(session.Device, SecureBondService.SERV_UUID)) throw new Exception("");

                    // Erase all bonds
                    if (!await SecureBondService.EraseAllBonds(session.Device)) throw new Exception("");                    

                    // Ensure System Configuration Service Exists
                    if (!await BLE_AdvFunctions.FindServiceAttributes(session.Device, SysCfgService.SERV_UUID)) throw new Exception("");

                    // Ensure Device Information Service Exisits
                    if (!await BLE_AdvFunctions.FindServiceAttributes(session.Device, DevInfoService.SERV_UUID)) throw new Exception("");

                    // Read all Device Information Service attributes
                    ConfigurationData cfgData = new ConfigurationData();
                    if (!await DevInfoService.ReadAllAtt(session.Device, cfgData)) throw new Exception("");

                    // Only finialise provisioning if necessary
                    ConfigurationData.eUseCase useCase = cfgData.GetUseCase();
                    if ((useCase == ConfigurationData.eUseCase.eBeacon) || (useCase == ConfigurationData.eUseCase.eAuthBeacon))
                    {
                        // Reset provision
                        if (!await SysCfgService.ResetProvision(session.Device)) throw new Exception("");
                    }
                }
                catch (Exception e)
                {
                    // Force an early close
                    session.Close();

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplIdle);

                    // Display error message
                    MessageBoxEx.Show((e.Message.Equals("") ? "Processing Timeout" : e.Message), "Status", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return false;
                }
            }

            // Post disconnect processing...

            // Update bond information to exporter
            m_exporter.RemoveBond(new BondInfo(bleDeviceData.Address));

            // Since bonds have changed, set flag to recommened bonds to be saved
            m_isSaveRecommended = true;

            // Clear device page
            clearDevicePage(bleDeviceData);

            // Update status bar
            UpdateStatus(eApplicationStatus.eApplIdle);

            // Enable provision button
            m_bleDeviceList.UpdateProvision(bleDeviceData, true);
            
            // Display result message
            MessageBoxEx.Show("Device has been reset", "Status", MessageBoxButtons.OK, MessageBoxIcon.Information);
            return true;
        }

        /// <summary>
        /// This type of bonding is only valid for authenticated beacons and proximity tokens
        /// </summary>
        /// <param name="bleDeviceData"></param>
        /// <returns></returns>
        private async Task<bool> PerformCreateBond(BleDeviceData bleDeviceData)
        {
            // Update status bar
            UpdateStatus(eApplicationStatus.eApplConnecting);

            // Start Session
            SharedSecret sharedSecret = new SharedSecret();
            ConfigurationData cfgData = new ConfigurationData();
            using (var session = new ConnectedSession(MiTokenBLE, bleDeviceData.Address, m_procedureCompletedEvent, m_connectionDisconnectedEvent))
            {
                try
                {
                    // Check device is connected
                    if (!await session.Open()) throw new Exception("");

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplProcess, "Creating bond", true);

                    // Update Connection Paramteres
                    if (!await BLE_AdvFunctions.UpdateConnectionParameters(session.Device, m_connectionUpdateResponseEvent, m_connectionUpdateResponseSemaphore)) throw new Exception("");

                    // Attempt to create a bond
                    IProgress<int> progressBarStatus = new Progress<int>(value => { toolStripProgressBar.Value = value; });
                    if (!await SecureBondService.CreateBond(session.Device, bleDeviceData.Address, sharedSecret, progressBarStatus, session.ProcedureCompletedEvent)) throw new Exception("Bond creation failed!\nPlease try again");

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplProcess, "Storing bond", false);

                    // Ensure Device Information Service Exists
                    if (!await BLE_AdvFunctions.FindServiceAttributes(session.Device, DevInfoService.SERV_UUID)) throw new Exception("");

                    // Ensure System Configuration Service Exists
                    if (!await BLE_AdvFunctions.FindServiceAttributes(session.Device, SysCfgService.SERV_UUID)) throw new Exception("");
                    
                    // Retrieve Configuration Data
                    if (!await DevInfoService.ReadAllAtt(session.Device, cfgData)) throw new Exception("");
                    if (!await SysCfgService.ReadAllAtt(session.Device, cfgData)) throw new Exception("");                    
                }
                catch (Exception e)
                {
                    Debug.WriteLine(String.Format("{0}\tLastError={1}", DateTime.Now, m_lastError.Value));                    

                    // Force an early close
                    session.Close();

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplIdle);

                    // Display error message
                    MessageBoxEx.Show((e.Message.Equals("") ? "Processing Timeout" : e.Message), "Status", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return false;
                }
            }

            // Post disconnect processing...

            // Disable provision button
            m_bleDeviceList.UpdateProvision(bleDeviceData, false);

            // Save bond information to exporter
            BondInfo bondInfo = new BondInfo(bleDeviceData, sharedSecret, DateTime.UtcNow, cfgData);
            m_exporter.AddBond(bondInfo);

            // Since bonds have changed, set flag to recommened bonds to be saved
            m_isSaveRecommended = true;

            // Clear device page
            clearDevicePage(bleDeviceData);

            // Update status bar
            UpdateStatus(eApplicationStatus.eApplIdle);

            // Display result message
            MessageBoxEx.Show("Bond created successfully!", "Status", MessageBoxButtons.OK, MessageBoxIcon.Information);            
            return true;
        }

        private async Task<bool> PerformAutoProvision(BleDeviceData bleDeviceData)
        {
#if DEBUG_DIAGNOSTICS
            Stopwatch sw = new Stopwatch();
            sw.Start();
#endif

            // Retrieve previous settings
            BeaconData beaconData = new BeaconData();
            beaconData.UUID = m_prevBeaconData.UUID;
            beaconData.Major = m_prevBeaconData.Major;
            beaconData.Minor = m_prevBeaconData.Minor;
            beaconData.Minor++; // Increment minor
            int advertIntervalMs = m_prevAdvertIntervalMs;

            // Retrieve custom beacon data from user
            FormAutoProv formAutoProv = new FormAutoProv(beaconData, advertIntervalMs);
            if (DialogResult.OK != formAutoProv.ShowDialog())
            {
                return false;
            }
            beaconData = formAutoProv.m_beaconData;
            advertIntervalMs = formAutoProv.m_advertIntervalMs;

            // Update status bar
            UpdateStatus(eApplicationStatus.eApplConnecting);

            // Start Session
            SharedSecret sharedSecret = new SharedSecret();
            ConfigurationData newCfgData = null;
            using (var session = new ConnectedSession(MiTokenBLE, bleDeviceData.Address, m_procedureCompletedEvent, m_connectionDisconnectedEvent))
            {
                try
                {
                    // Check device is connected
                    if (!await session.Open()) throw new Exception("");

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplProcess, "Checking status: Services");

                    // Update Connection Paramteres
                    if (!await BLE_AdvFunctions.UpdateConnectionParameters(session.Device, m_connectionUpdateResponseEvent, m_connectionUpdateResponseSemaphore)) throw new Exception("");

                    // Ensure Device Information Service Exists
                    if (!await BLE_AdvFunctions.FindServiceAttributes(session.Device, DevInfoService.SERV_UUID)) throw new Exception("");

                    // Ensure System Configuration Service Exists
                    if (!await BLE_AdvFunctions.FindServiceAttributes(session.Device, SysCfgService.SERV_UUID)) throw new Exception("");

                    // Update with device address
                    ConfigurationData oldCfgData = new ConfigurationData();
                    oldCfgData.Address = bleDeviceData.Address;

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplProcess, "Checking status: Configuration");

                    // Retrieve Configuration Data
                    if (!await DevInfoService.ReadAllAtt(session.Device, oldCfgData)) throw new Exception("");
                    if (!await SysCfgService.ReadAllAtt(session.Device, oldCfgData)) throw new Exception("");     

                    // No longer need to check Firmware Use Case is correct as done when popluating control buttons
                    // Check if device has already been provisioned
                    if (oldCfgData.IsProvisioned) throw new Exception("Device already provisioned!");

                    // Create copy before amending new configuration data to apply
                    newCfgData = new ConfigurationData(oldCfgData);
                    newCfgData.UUID = beaconData.UUID;
                    newCfgData.Major = beaconData.Major;
                    newCfgData.Minor = beaconData.Minor;
                    newCfgData.AdvertisingIntervalMs = advertIntervalMs; // Note: Legacy as of V1.11 but kept for backward compatibility
                    newCfgData.AdvertisingStageSettings.SetIdleTimeoutMs(ConfigurationData.AdvSmStageSettings.Stages.Stage0, Convert.ToUInt16(ConfigurationData.AdvSmStageSettings.MIN_ADVSM_IDLETIMEOUT_MS));
                    newCfgData.AdvertisingStageSettings.SetIdleTimeoutMs(ConfigurationData.AdvSmStageSettings.Stages.Stage1, Convert.ToUInt16(advertIntervalMs)); // Must be same as interval to pass validation
                    newCfgData.AdvertisingStageSettings.SetIntervalMs(ConfigurationData.AdvSmStageSettings.Stages.Stage1, Convert.ToUInt16(advertIntervalMs));
                    newCfgData.AdvertisingStageSettings.SetIdleTimeoutMs(ConfigurationData.AdvSmStageSettings.Stages.Stage2, Convert.ToUInt16(ConfigurationData.AdvSmStageSettings.MIN_ADVSM_IDLETIMEOUT_MS));
                    newCfgData.AdvertisingStageSettings.SetIntervalMs(ConfigurationData.AdvSmStageSettings.Stages.Stage2, Convert.ToUInt16(ConfigurationData.AdvSmStageSettings.MIN_ADVSM_INTERVAL_MS));
                    newCfgData.AdvertisingStageSettings.SetIdleTimeoutMs(ConfigurationData.AdvSmStageSettings.Stages.Stage3, Convert.ToUInt16(ConfigurationData.AdvSmStageSettings.MIN_ADVSM_IDLETIMEOUT_MS));
                    newCfgData.AdvertisingStageSettings.SetIntervalMs(ConfigurationData.AdvSmStageSettings.Stages.Stage3, Convert.ToUInt16(ConfigurationData.AdvSmStageSettings.MIN_ADVSM_INTERVAL_MS));

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplProcess, "Creating bond", true);

                    // Attempt to create a bond                    
                    IProgress<int> progressBarStatus = new Progress<int>(value => { toolStripProgressBar.Value = value; });
                    if (!await SecureBondService.CreateBond(session.Device, bleDeviceData.Address, sharedSecret, progressBarStatus, session.ProcedureCompletedEvent)) throw new Exception("Could not create a bond!");                    

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplProcess, "Creating a secure session", true);

                    // Use bond's shared secret to secure a session
                    SessionData sessionData = await AuthenService.EstablishSecureSession(session.Device, m_connectionUpdateResponseEvent, m_connectionUpdateResponseSemaphore, bleDeviceData.Address, sharedSecret, progressBarStatus, session.ProcedureCompletedEvent);
                    if (!sessionData.IsAuthenticated) throw new Exception("Could not establish a session");

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplProcess, "Updating configuration");

                    // Update affected configuration
                    if (!await SysCfgService.UpdateConfiguration(session.Device, oldCfgData, newCfgData, sessionData)) throw new Exception("");

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplProcess, "Finalizing provision");

                    // Finialize provision
                    if (!await SysCfgService.FinializeProvision(session.Device, sessionData)) throw new Exception("");
                }
                catch (Exception e)
                {
                    // Force an early close
                    session.Close();

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplIdle);

                    // Display error message
                    MessageBoxEx.Show((e.Message.Equals("") ? "Processing Timeout" : e.Message), "Status", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return false;
                }
#if DEBUG_DIAGNOSTICS
                finally
                {
                    sw.Stop();
                    Debug.WriteLine(String.Format("{0}\tAutoProvisionDevice:ElapsedTime=[{1}]", DateTime.Now, sw.Elapsed));
                }
#endif
            }

            // Post disconnect processing...

            // Disable provision button
            m_bleDeviceList.UpdateProvision(bleDeviceData, false);

            // Update Beacon Data before exporting
            bleDeviceData.BeaconData = beaconData;

            // Save bond information to exporter
            BondInfo bondInfo = new BondInfo(bleDeviceData, sharedSecret, DateTime.UtcNow, newCfgData);
            m_exporter.AddBond(bondInfo);

            // Since bonds have changed, set flag to recommened bonds to be saved
            m_isSaveRecommended = true;

            // Save beacon data for next device to auto-provision
            m_prevBeaconData = beaconData;
            m_prevAdvertIntervalMs = advertIntervalMs;

            // Clear device page
            clearDevicePage(bleDeviceData);

            // Update status bar
            UpdateStatus(eApplicationStatus.eApplIdle);

            // Display result message
            MessageBoxEx.Show("Auto-Provisioning successful!", "Status", MessageBoxButtons.OK, MessageBoxIcon.Information);

#if DEBUG_DIAGNOSTICS
            sw.Stop();
            Debug.WriteLine(String.Format("{0}\tAutoProvisionDevice:ElapsedTime=[{1}]", DateTime.Now, sw.Elapsed));
#endif
            return true;
        }

        private async Task<bool> PerformOAD(BleDeviceData bleDeviceData)
        {
            // Update status bar
            UpdateStatus(eApplicationStatus.eApplConnecting);

            // Start Session
            OadService oadService = new OadService();
            using (var session = new ConnectedSession(MiTokenBLE, bleDeviceData.Address, m_procedureCompletedEvent, m_connectionDisconnectedEvent))
            {
                try
                {
                    // Check device is connected
                    if (!await session.Open()) throw new Exception("");

                    // Update status bar
                    UpdateStatus(eApplicationStatus.eApplProcess, "Updating firmware");

                    // Update Connection Paramteres
                    if (!await BLE_AdvFunctions.UpdateConnectionParameters(session.Device, m_connectionUpdateResponseEvent, m_connectionUpdateResponseSemaphore)) throw new Exception("");

                    // Retrieve target header
                    if (!await Task.Run(() => oadService.RetrieveTargetImgHdr(session.Device))) throw new TimeoutException();

                    // Load OAD Form
                    if (oadService.getTargetImgHdr() == null) throw new TimeoutException();
                    FormOad formOad = new FormOad(oadService, session.Device, session.ConnectionDisconnectedEvent);
                    formOad.ShowDialog();
                }
                catch (Exception e)
                {
                    MessageBoxEx.Show((e.Message.Equals("") ? "Processing Timeout" : e.Message), "Status", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return false;
                }
            }

            // Post disconnect processing...

            // Clear device page
            clearDevicePage(bleDeviceData);
            return true;
        }

        private void serialPortToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // Disable current form
            this.Enabled = false;

            // Stop scanning
            StopBleScanning();

            // Display as modal to ensure a com port is selected
            if (m_formComPort.ShowDialog() == DialogResult.OK)
            {                
                // Ensure to disconnect from previous serial
                DisconnectFromSerial();

                // Connect to Serial
                m_serialPortStatus = ConnectToSerial();
                if (m_serialPortStatus == BLE.Core.API_RET.BLE_API_SUCCESS)
                {
                    // Save COM Port to registry only if sucessfully opened
                    m_formComPort.SaveToRegistry();

                    // Start scanning
                    StartBleScanning();
                }
            }
            else
            {
                // Start scanning on same port
                StartBleScanning();
            }

            // Re-enable current form
            this.Enabled = true;
        }

        private void FormProvTool_KeyDown(object sender, KeyEventArgs e)
        {
            // Need to supress this to prevent exit out of the form early
            if (e.KeyCode == Keys.Enter)
            {
                e.SuppressKeyPress = true;
            }
        }

        private void provisioningToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // Disable current form
            this.Enabled = false;

            // Stop scanning
            StopBleScanning();

            StreamWriter file;
            SaveFileDialog saveFileDialog = new SaveFileDialog();
            saveFileDialog.Filter = "Text Documents (*.txt)|*.txt";
            saveFileDialog.FilterIndex = 1;
            saveFileDialog.RestoreDirectory = true;

            // Retrieve file name
            if (saveFileDialog.ShowDialog() == DialogResult.OK)
            {
                try
                {
                    file = new System.IO.StreamWriter(saveFileDialog.OpenFile());
                    File.SetAttributes(saveFileDialog.FileName, FileAttributes.ReadOnly);
                    file.Write(m_exporter.GetStringAll()); // Write all bond information to file
                    file.Close();

                    MessageBoxEx.Show("File (Basic) Saved", "Status", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
                catch
                {
                    MessageBoxEx.Show("Could not open file", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }

            // Dispose dialog
            saveFileDialog.Dispose();

            // Start scanning
            StartBleScanning();

            // Re-enable current form
            this.Enabled = true;  
        }

        private void extendedToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // Disable current form
            this.Enabled = false;

            // Stop scanning
            StopBleScanning();

            StreamWriter file;
            SaveFileDialog saveFileDialog = new SaveFileDialog();
            saveFileDialog.Filter = "Text Documents (*.txt)|*.txt";
            saveFileDialog.FilterIndex = 1;
            saveFileDialog.RestoreDirectory = true;

            // Retrieve file name
            if (saveFileDialog.ShowDialog() == DialogResult.OK)
            {
                try
                {
                    file = new System.IO.StreamWriter(saveFileDialog.OpenFile());
                    File.SetAttributes(saveFileDialog.FileName, FileAttributes.ReadOnly);
                    file.Write(m_exporter.GetStringExtendedAll()); // Write all extended bond information to file
                    file.Close();

                    // Update flag as all current bonds have been saved
                    m_isSaveRecommended = false;

                    MessageBoxEx.Show("File (Extended) Saved", "Status", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
                catch
                {
                    MessageBoxEx.Show("Could not open file", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }

            // Dispose dialog
            saveFileDialog.Dispose();

            // Start scanning
            StartBleScanning();

            // Re-enable current form
            this.Enabled = true;  
        }

        private void SetDevNameFilterMenuItems(DevNameFilterType.eType type)
        {
            foreach (KeyValuePair<DevNameFilterType.eType, ToolStripMenuItem> entry in m_filterTypeDictionary)
            {
                if (entry.Key == type)
                {
                    entry.Value.Checked = true;
                }
                else
                {
                    entry.Value.Checked = false;
                }
            }
        }

        private void SetDevNameFilterType(DevNameFilterType.eType type)
        {
            // Ensure correct menu items are checked
            SetDevNameFilterMenuItems(type);

            // Stop Scanning
            StopBleScanning();

            // Set filter type and save to registry
            m_devNameFilterType.Type = type;
            m_devNameFilterType.SaveToRegistry();

            // Clear device page
            clearDevicePage();

            // Start scanning again
            StartBleScanning();
        }

        private void noneToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SetDevNameFilterType(DevNameFilterType.eType.None);
        }

        private void allMiTokenToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SetDevNameFilterType(DevNameFilterType.eType.MtAll);
        }

        private void miTokenBluToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SetDevNameFilterType(DevNameFilterType.eType.MtBlu);
        }

        private void miTokenBeaconToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SetDevNameFilterType(DevNameFilterType.eType.MtBcn);
        }

        private void miTokenBootToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SetDevNameFilterType(DevNameFilterType.eType.MtBoot);
        }

        private void SetMenuStipEnabled(bool isEnabled)
        {
            exportToolStripMenuItem.Enabled = isEnabled;
            settingsToolStripMenuItem.Enabled = isEnabled;
        }

        private void refreshToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // Stop Scanning
            StopBleScanning();

            // Clear device page
            clearDevicePage();

            // Start scanning again
            StartBleScanning();
        }

        private void clearDevicePage(BleDeviceData bleDeviceData = null)
        {
            // Clear configuration settings
            ucConfigurationData.Reset();

            if (bleDeviceData != null)
            {
                m_bleDeviceList.Remove(ref flowLayoutPanelBleDevices, bleDeviceData);
            }
            else
            {
                // Clear device selection tab
                m_bleDeviceList.Reset();

                // Reset layout
                flowLayoutPanelBleDevices.Controls.Clear();
            }
        }

        public class LastError
        {
            
            private Mutex m_mutex = new Mutex();
            private int m_lastError = 0;

            public int Value
            {
                get
                {
                    int temp = 0;
                    m_mutex.WaitOne();
                    temp = m_lastError;
                    m_mutex.ReleaseMutex();
                    return temp;
                }

                set
                {
                    m_mutex.WaitOne();
                    m_lastError = value;
                    m_mutex.ReleaseMutex();
                }
            }
        }

        static public BLE.CommandChain.CommandChainCall ProcedureCompletedEvent = (IntPtr caller, IntPtr data) =>
        {
            BLE.V2_2.MiTokenBLE bleObject = BLE.Core.LookupOnIntPtrV2_2(caller);
            if (bleObject == null)
            {
                return;
            }

            if (bleDictionary.ContainsKey(bleObject))
            {
                var msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.ATTCLIENT.PROCEDURE_COMPLETED(data);
                bleDictionary[bleObject].m_lastError.Value = msg.result;
                bleDictionary[bleObject].m_procedureCompletedEvent.Set();
            }
        };

        static public BLE.CommandChain.CommandChainCall ConnectionStatusEvent = (IntPtr caller, IntPtr data) =>
        {
            BLE.V2_2.MiTokenBLE bleObject = BLE.Core.LookupOnIntPtrV2_2(caller);
            if (bleObject == null)
            {
                return;
            }

            if (bleDictionary.ContainsKey(bleObject))
            {
                var msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.CONNECTION.STATUS(data);
                if (msg.flags == 1)
                {
                    bleObject.ForceUnsafeDisconnection(msg.connection);
                }
            }
        };

        static public BLE.CommandChain.CommandChainCall ConnectionUpdateResponse = (IntPtr caller, IntPtr data) =>
        {
            Debug.WriteLine(String.Format("{0}\tconnection_update_rsp:ENTER", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt")));
            BLE.V2_2.MiTokenBLE bleObject = BLE.Core.LookupOnIntPtrV2_2(caller);
            if (bleObject == null)
            {
                return;
            }

            bleDictionary[bleObject].m_connectionUpdateResponseSemaphore.Wait();
                        
            if (bleDictionary.ContainsKey(bleObject))
            {                    
                var msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.CONNECTION.UPDATE(data);
                bleDictionary[bleObject].m_connectionUpdateResponseEvent.Set();
                Debug.WriteLine(String.Format("{0}\tconnection_update_rsp:SET result={1}", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt"), msg.result));
            }
            bleDictionary[bleObject].m_connectionUpdateResponseSemaphore.Release();
            Debug.WriteLine(String.Format("{0}\tconnection_update_rsp:EXIT", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt")));
        };

        static public BLE.CommandChain.CommandChainCall ConnectionDisconnectedEvent = (IntPtr caller, IntPtr data) =>
        {
            BLE.V2_2.MiTokenBLE bleObject = BLE.Core.LookupOnIntPtrV2_2(caller);
            if (bleObject == null)
            {
                return;
            }

            if (bleDictionary.ContainsKey(bleObject))
            {
                bleDictionary[bleObject].m_connectionDisconnectedEvent.Set();
            }
        };

        private void bulkOADToolStripMenuItem_Click(object sender, EventArgs e)
        {
            using (var uiSession = new UISession(this))
            {
                // Update status bar
                UpdateStatus(eApplicationStatus.eApplProcess, "Bulk Update");

                using (FormBulkOAD f = new FormBulkOAD(this))
                {
                    // Display form
                    f.ShowDialog();

                    // Clear device page
                    clearDevicePage();
                }
            }
        }
    }    
}

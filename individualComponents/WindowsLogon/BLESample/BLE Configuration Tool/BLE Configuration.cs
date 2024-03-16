using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Microsoft.Win32;

using System.Net;
using System.Net.Security;
using System.Security.Cryptography.X509Certificates;

using System.Threading;
using BLE_API;
using System.Management;
using System.Reflection;
using System.Diagnostics;

namespace BLE_Configuration_Tool
{
    public partial class BLE_Configuration : Form
    {
        private RegistryKey getRegKey(bool writeAccess = false)
        {
            return Registry.LocalMachine.OpenSubKey(@"Software\Mi-Token\BLE\", writeAccess);
        }

        BLE.V2_2.MiTokenBLE MiTokenBLE = null;
        bool isSetupComplete = false;
        
        public BLE_Configuration()
        {
            try
            {
                InitializeComponent();
                curStageData = new StageCOM(this); //StageCOM will initialize everything we need
                rootStageData = curStageData;

                RegistryKey regKey = getRegKey();
                // Doesn't exisit so create
                if (regKey == null)
                {
                    regKey = Registry.LocalMachine.CreateSubKey(@"Software\Mi-Token\BLE");
                }
                curStageData.LoadFromRegistry(regKey);
                regKey.Close();

                curStageData.setAsActive();
                setToStage();
                stageRefresh();

                if (ServicePointManager.ServerCertificateValidationCallback == null)
                {
                    ServicePointManager.ServerCertificateValidationCallback += new System.Net.Security.RemoteCertificateValidationCallback(ValidateRemoteCertificate);
                }
            }

            catch (Exception ex)
            {
                MessageBox.Show(this, ex.Message + "\r\n" + ex.StackTrace, "Exception", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        bool testingRSSILevels = false;
        int pollSinceLastSeen = 0;
        int pollWithInvalidRSSI = 0;
        DevicePollConfig DPC = null;
        string DPCDeviceMac = "";

        private static bool ValidateRemoteCertificate(object sender, X509Certificate certificate, X509Chain chain, SslPolicyErrors policyErrors)
        {
            return true; //just allow everything
        }

        abstract class StageData
        {
            public abstract bool allowNext();
            public abstract bool allowPrev();
            public bool isFirst
            {
                get
                {
                    return _prevStage == null;
                }
            }
            public bool isLast
            {
                get
                {
                    return _nextStage == null;
                }
            }

            public StageData GetStageOfType(Type type)
            {
                if (this.GetType() == type)
                {
                    return this;
                }
                else if (_nextStage != null)
                {
                    return _nextStage.GetStageOfType(type);
                }
                else
                {
                    return null;
                }
            }

            public BLE_Configuration _linkedForm;

            public virtual bool next(ref StageData stage)
            {
                if (allowNext())
                {
                    stage = _nextStage;
                    return true;
                }
                return false;
            }
            public virtual bool prev(ref StageData stage)
            {
                if (allowPrev())
                {
                    stage = _prevStage;
                    return true;
                }
                return false;
            }

            public abstract void setAsActive();

            protected StageData _nextStage;
            protected StageData _prevStage;

            public abstract void SaveToRegistry(RegistryKey RegKey);
            public abstract void LoadFromRegistry(RegistryKey RegKey);
        };


        StageData curStageData = null;
        StageData rootStageData = null;

        public void stageRefresh()
        {
            if(InvokeRequired)
            {
                Invoke((MethodInvoker)delegate
                {
                    stageRefresh();
                });
                return; 
            }

            butBack.Enabled = curStageData.allowPrev();
            butNext.Enabled = curStageData.allowNext();

            if (curStageData.isLast)
            {
                butNext.Enabled = true;
                butNext.Text = "Save and Quit";
            }
            else
            {
                butNext.Text = "Next";
            }
        }

        class StageUserPreferences : StageData
        {
            public StageUserPreferences(StageData prevStage, BLE_Configuration form)
            {
                _prevStage = prevStage;
                _nextStage = new StageAPIServers(this, form);
                _linkedForm = form;
            }
            public override bool allowNext()
            {
                return true;
            }
            public override bool allowPrev()
            {
                return true;
            }

            public override void setAsActive()
            {
                _linkedForm.curStage = ConfigStage.UserPreferences;
                _linkedForm.Size = new Size(445, 420);
                if (IsUserMulti)
                    _linkedForm.radioButtonUserMulti.Select();
                else
                    _linkedForm.radioButtonUserSingle.Select();
            }

            public override void SaveToRegistry(RegistryKey RegKey)
            {
                RegKey.SetValue("IsUserMulti", BitConverter.GetBytes(IsUserMulti), RegistryValueKind.Binary);

                if (_nextStage != null)
                {
                    _nextStage.SaveToRegistry(RegKey);
                }
            }

            private bool loadByteFromReg(RegistryKey RegKey, string KeyName, ref bool output)
            {
                return (bool)RegKey.GetValue(KeyName, null);
            }

            public override void LoadFromRegistry(RegistryKey RegKey)
            {
                
                IsUserMulti = BitConverter.ToBoolean((byte[])RegKey.GetValue("IsUserMulti", new byte[] {0}), 0);

                if (_nextStage != null)
                {
                    _nextStage.LoadFromRegistry(RegKey);
                }
            }

            public bool IsUserMulti = false; // Default to single user
        }

        class StageConfigSummary : StageData
        {
            public StageConfigSummary(StageData prevStage, BLE_Configuration form)
            {
                _prevStage = prevStage;
                _nextStage = null;
                _linkedForm = form;
            }
            public override bool allowNext()
            {
                return false;
            }
            public override bool allowPrev()
            {
                return true;
            }
            public override void setAsActive()
            {
                _linkedForm.curStage = ConfigStage.ConfigSummary;
                _linkedForm.Size = new Size(445, 420);

                StageCOM com = _linkedForm.getCOMData();
                _linkedForm.labCOMPort.Text = "COM Port : " + com.COMPort;

                _linkedForm.labDeviceName.Text = "Device Name : " + _linkedForm.getDeviceFilterData().DeviceName;
                _linkedForm.labPrimaryMiToken.Text = "Primary Mi-Token API Server : " + _linkedForm.getAPIServersData().APIServers[0];

                BLE_Configuration.ServiceState serviceState = _linkedForm.getServiceState();

                string sstate = "";
                switch (serviceState)
                {
                    case ServiceState.Uninstalled:
                        sstate = "Not Installed";
                        _linkedForm.butServiceStartStop.Enabled = false;
                        _linkedForm.butServiceStartStop.Text = "Start Serivce";
                        break;
                    case ServiceState.InstalledAndRunnig:
                        sstate = "Running";
                        _linkedForm.butServiceStartStop.Enabled = true;
                        _linkedForm.butServiceStartStop.Text = "Stop Serivce";
                        break;
                    case ServiceState.InstalledAndStopped:
                        sstate = "Stopped";
                        _linkedForm.butServiceStartStop.Enabled = true;
                        _linkedForm.butServiceStartStop.Text = "Start Serivce";
                        break;
                    case ServiceState.InstalledAndUnknown:
                        sstate = "Installed and in unknown";
                        _linkedForm.butServiceStartStop.Enabled = true;
                        _linkedForm.butServiceStartStop.Text = "Start Serivce";
                        break;
                    default:
                        sstate = "Unknown ServiceState";
                        _linkedForm.butServiceStartStop.Enabled = true;
                        _linkedForm.butServiceStartStop.Text = "Start Serivce";
                        break;
                }

                _linkedForm.labServiceStatus.Text = "Service Status : " + sstate;
            }

            public override void SaveToRegistry(RegistryKey RegKey)
            {
                if (_nextStage != null)
                {
                    _nextStage.SaveToRegistry(RegKey);
                }
            }

            public override void LoadFromRegistry(RegistryKey RegKey)
            {
                if (_nextStage != null)
                {
                    _nextStage.LoadFromRegistry(RegKey);
                }
            }
        };
        class StageBonding : StageData
        {
            public StageBonding(StageData prevStage, BLE_Configuration form)
            {
                _prevStage = prevStage;
                _nextStage = new StageConfigSummary(this, form);
                _linkedForm = form;
            }
            public override bool allowNext()
            {
                return true;
            }
            public override bool allowPrev()
            {
                return true;
            }
            public override void setAsActive()
            {
                _linkedForm.curStage = ConfigStage.BondingStage;
                _linkedForm.Size = new Size(445, 420);
                if (_linkedForm.txtBondUser.Text != "")
                    _linkedForm.butBondToDevice.Enabled = true;
                else
                    _linkedForm.butBondToDevice.Enabled = false;
            }

            public override void SaveToRegistry(RegistryKey RegKey)
            {
                RegKey.SetValue("BondDeviceUser", _linkedForm.txtBondUser.Text);

                if (_nextStage != null)
                {
                    _nextStage.SaveToRegistry(RegKey);
                }
            }
            public override void LoadFromRegistry(RegistryKey RegKey)
            {
                string bondUser = (string)RegKey.GetValue("BondDeviceUser", "");
                _linkedForm.txtBondUser.Text = bondUser;

                if (_nextStage != null)
                {
                    _nextStage.LoadFromRegistry(RegKey);
                }
            }
        };

        class StageLocationCalib : StageData
        {
            public StageLocationCalib(StageData prevStage, BLE_Configuration form)
            {
                _prevStage = prevStage;
                _nextStage = new StageBonding(this, form);
                _linkedForm = form;
            }
            public override bool allowNext()
            {
                return setup;
            }
            public override bool allowPrev()
            {
                return true;
            }
            public override void setAsActive()
            {
                _linkedForm.curStage = ConfigStage.LocationCalib;
                _linkedForm.Size = new Size(445, 420);
                _linkedForm.progStageProgress.Value = 0;
                _linkedForm.labUnlockRSSI.Text = "Login Signal: " + ConvertRssidBmToSignalQuality(unlockRSSI).ToString() + "%";
                _linkedForm.labLockRSSI.Text = "Lock Signal: " + ConvertRssidBmToSignalQuality(lockRSSI).ToString() + "%";
                _linkedForm.labLatestRSSI.Text = "Latest Signal: -";
                _linkedForm.StartLocationCalibration();
            }

            public override void SaveToRegistry(RegistryKey RegKey)
            {
                RegKey.SetValue("LogoffRSSI", new byte[] { lockRSSI }, RegistryValueKind.Binary);
                RegKey.SetValue("LogonRSSI", new byte[] { unlockRSSI }, RegistryValueKind.Binary);

                if (_nextStage != null)
                {
                    _nextStage.SaveToRegistry(RegKey);
                }

                using (var key = Registry.LocalMachine.OpenSubKey("Software\\Microsoft\\Windows\\CurrentVersion\\Run", true))
                {
                    if (key != null)
                    {
                        key.SetValue("BLE Watcher", "\"" + AppDomain.CurrentDomain.BaseDirectory + "BLELockout.exe\"", RegistryValueKind.String);
                    }
                }
            }

            private bool loadByteFromReg(RegistryKey RegKey, string KeyName, ref byte output)
            {
                byte[] bin = (byte[])RegKey.GetValue(KeyName, null);
                if (bin == null)
                {
                    output = 0;
                    return false;
                }
                else
                {
                    output = bin[0];
                    return true;
                }
            }

            public override void LoadFromRegistry(RegistryKey RegKey)
            {
                bool loadSuccess = true;
                loadSuccess &= (loadByteFromReg(RegKey, "LogoffRSSI", ref lockRSSI));
                loadSuccess &= (loadByteFromReg(RegKey, "LogonRSSI", ref unlockRSSI));

                setup = loadSuccess;

                if (_nextStage != null)
                {
                    _nextStage.LoadFromRegistry(RegKey);
                }
            }

            public byte lockRSSI = 0, unlockRSSI = 0;
            public bool setup = false;
            

        };
        class StageAPIServers : StageData
        {
            public StageAPIServers(StageData prevStage, BLE_Configuration form)
            {
                _prevStage = prevStage;
                _nextStage = new StageDeviceFilter(this, form);
                _linkedForm = form;
            }
            public override bool allowNext()
            {
                return (APIServers.Count > 0);
            }
            public override bool allowPrev()
            {
                return true;
            }

            public override void setAsActive()
            {
                _linkedForm.curStage = ConfigStage.APIServers;
                _linkedForm.Size = new Size(445, 420);
                _linkedForm.txtMiTokenAPIServer.Clear();
                _linkedForm.butValidateMiTokenAPIServer.Enabled = false;
                _linkedForm.butRemoveMiTokenAPIServer.Enabled = false;
            }

            public override void SaveToRegistry(RegistryKey RegKey)
            {
                RegKey.SetValue("APIServers", APIServers.ToArray(), RegistryValueKind.MultiString);

                if (_nextStage != null)
                {
                    _nextStage.SaveToRegistry(RegKey);
                }
            }
            public override void LoadFromRegistry(RegistryKey RegKey)
            {
                string[] servers = (string[])RegKey.GetValue("APIServers", null);
                if (servers != null)
                {
                    APIServers = new List<string>();
                    foreach (string s in servers)
                    {
                        APIServers.Add(s);
                    }
                }

                if (_nextStage != null)
                {
                    _nextStage.LoadFromRegistry(RegKey);
                }

                _linkedForm.lstAPIServers.Items.Clear();
                foreach (string s in APIServers)
                {
                    _linkedForm.lstAPIServers.Items.Add(s);
                }
            }

            public List<string> APIServers = new List<string>();

        };       
        class StageDeviceFilter : StageData
        {
            public StageDeviceFilter(StageData prevStage, BLE_Configuration form)
            {
                _prevStage = prevStage;
                _nextStage = new StageLocationCalib(this, form);
                _linkedForm = form;                
            }

            public override bool allowNext()
            {
                if (DeviceMacToMonitor != "")
                {
                    return true;
                }
                return false;
            }
            public override bool allowPrev()
            {
                return true;
            }
            
            public override void setAsActive()
            {
                _linkedForm.curStage = ConfigStage.DeviceFilter;
                _linkedForm.Size = new Size(445, 420);
                _linkedForm.lstDevices.ClearSelected();
                DeviceMacToMonitor = "";
            }

            public override void SaveToRegistry(RegistryKey RegKey)
            {
                RegKey.SetValue("DeviceName", DeviceName);

#if false // Remove MAC address and Meta Data filtering
                BLE.V2_1.Filter filter = _linkedForm.MiTokenBLE.CreateFilter();
                if (DeviceName != "")
                {
                    filter.AddFilter((char)9, Encoding.ASCII.GetBytes(DeviceName));
                }
                else
                {
                    filter.AddFilter((char)0, Encoding.ASCII.GetBytes(""));
                }

                int devCount = _linkedForm.MiTokenBLE.GetDeviceFoundCount();
                BLE.Core.Structures.DeviceInfo devInfo = new BLE.Core.Structures.DeviceInfo();
                bool found = false;
                for (int i = 0; (i < devCount) && (!found); ++i)
                {
                    _linkedForm.MiTokenBLE.GetDeviceInfo(i, ref devInfo);
                    if (devInfo.macString() == DeviceMacToMonitor)
                    {
                        found = true;
                        break;
                    }
                }
                if (found)
                {
                    filter.AddAddressFilter(devInfo.macAddress);
                }

                filter.FinalizeFilter();
                BLE.V2_1.Filter.Searcher searcher = filter.createNewSearcher();
                byte[] byteStream = searcher.GetByteStreamFromSearcher();

                RegKey.SetValue("FilterStream", byteStream, RegistryValueKind.Binary);
#endif

                if (_nextStage != null)
                {
                    _nextStage.SaveToRegistry(RegKey);
                }
            }
            public override void LoadFromRegistry(RegistryKey RegKey)
            {
                DeviceName = (string)RegKey.GetValue("DeviceName", "");

                if (_nextStage != null)
                {
                    _nextStage.LoadFromRegistry(RegKey);
                }

                _linkedForm.txtDeviceName.Text = DeviceName;
            }

            public string DeviceName = "";
            public string DeviceMacToMonitor = "";

        };
        class StageCOM : StageData
        {
            public StageCOM(BLE_Configuration form)
            {
                _prevStage = null;
                _nextStage = new StageUserPreferences(this, form);
                _linkedForm = form;
                COMPort = "";
                tempPort = "";
            }

            public override bool allowNext()
            {
                if ((COMPort != "") && isCOMPortTested)
                {
                    if (!isCOMPortTested)
                    {
                        return false;
                    }
                    else
                    {
                        return true;
                    }
                }
                return false;
            }
            public override bool allowPrev()
            {
                return false; //never allow prev
            }

            public override bool next(ref StageData stage)
            {
                if (allowNext())
                {
                    return base.next(ref stage);
                }
                return false;
            }

            

            public override void setAsActive()
            {
                _linkedForm.curStage = ConfigStage.COMSelect;

                //change form to recommended size
                _linkedForm.Size = new Size(445, 420);

                _linkedForm.labelCOMStatus.Text = GetCOMStatus();
                
                _linkedForm.butRefreshCOM_Click(null, new EventArgs());
                //TOOD Fix?
                /*
                if (BLE_API_DLL.DLLWrapper.V2.IsInitialized)
                {
                    BLE_API_DLL.DLLWrapper.V2.Finalize();
                }
                 * */
            }

            public string COMPort;
            public string tempPort;

            public string GetCOMStatus()
            {
                if ((COMPort == "") && (tempPort == ""))
                    return "-";
                else
                {
                    if (isCOMPortTested)
                        return "Connected";
                    else
                        return "Disconnected";
                }
                
            }

            public override void SaveToRegistry(RegistryKey RegKey)
            {
                RegKey.SetValue("COMPort", COMPort);

                if (_nextStage != null)
                {
                    _nextStage.SaveToRegistry(RegKey);
                }
            }

            public override void LoadFromRegistry(RegistryKey RegKey)
            {
                COMPort = (string)RegKey.GetValue("COMPort", "");



                if (_nextStage != null)
                {
                    _nextStage.LoadFromRegistry(RegKey);
                }
            }

            public bool isCOMPortTested = false;
        }

        public static BLE.CommandChain.CommandChainCall ccc_evt_connection_disconnect = (IntPtr caller, IntPtr data) =>
            {
                BLE.V2_2.MiTokenBLE bleObject = BLE.Core.LookupOnIntPtrV2_2(caller);
                if (bleObject == null)
                {
                    return;
                }

                if (configDictionary.ContainsKey(bleObject))
                {
                    configDictionary[bleObject].disconnectionEvent.Set();
                }
            };


        public static BLE.CommandChain.CommandChainCall ccc_rsp_connection_update = (IntPtr caller, IntPtr data) =>
            {
                Debug.WriteLine(String.Format("{0}\tccc_rsp_connection_update:ENTER", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt")));

                BLE.V2_2.MiTokenBLE bleObject = BLE.Core.LookupOnIntPtrV2_2(caller);
                if (bleObject == null)
                {
                    return;
                }                

                configDictionary[bleObject].connectionUpdateResponseSemaphore.Wait();

                if (configDictionary.ContainsKey(bleObject))
                {
                    var msg = new BLE.CommandChain.CommandChainMessageTypes.RSP.CONNECTION.UPDATE(data);
                    if (msg.result == 0)
                    {
                        Debug.WriteLine(String.Format("{0}\tccc_rsp_connection_update:SET connection={1} result={2}", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt"), msg.connection, msg.result));
                        configDictionary[bleObject].connectionUpdateResponseEvent.Set();
                    }
                }

                configDictionary[bleObject].connectionUpdateResponseSemaphore.Release();

                Debug.WriteLine(String.Format("{0}\tccc_rsp_connection_update:EXIT", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt")));
            };

        public static BLE.CommandChain.CommandChainCall ccc_evt_procedure_completed = (IntPtr caller, IntPtr data) =>
            {
                Debug.WriteLine(String.Format("{0}\tccc_evt_procedure_completed:ENTER", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt")));

                BLE.V2_2.MiTokenBLE bleObject = BLE.Core.LookupOnIntPtrV2_2(caller);
                if (bleObject == null)
                {
                    return;
                }

                if (configDictionary.ContainsKey(bleObject))
                {
                    var msg = new BLE.CommandChain.CommandChainMessageTypes.EVT.ATTCLIENT.PROCEDURE_COMPLETED(data);
                    if (msg.result == 0)
                    {
                        Debug.WriteLine(String.Format("{0}\tccc_evt_procedure_completed:SET connection={1} result={2}", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt"), msg.connection, msg.result));
                        configDictionary[bleObject].procedureCompletedEvent.Set();
                    }
                }

                Debug.WriteLine(String.Format("{0}\tccc_evt_procedure_completed:EXIT", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt")));
            };

        public static Dictionary<BLE.V2_2.MiTokenBLE, BLE_Configuration> configDictionary = new Dictionary<BLE.V2_2.MiTokenBLE, BLE_Configuration>();
        AutoResetEvent disconnectionEvent = new AutoResetEvent(false);
        ManualResetEventSlim connectionUpdateResponseEvent = new ManualResetEventSlim(false);
        SemaphoreSlim connectionUpdateResponseSemaphore = new SemaphoreSlim(2);
        ManualResetEventSlim procedureCompletedEvent = new ManualResetEventSlim(false);

        public enum ConfigStage
        {
            COMSelect,
            UserPreferences,
            DeviceFilter,
            APIServers,
            LocationCalib,
            BondingStage,
            ConfigSummary,
        };

        public ConfigStage curStage = ConfigStage.COMSelect;

        private void setToStage()
        {
            groupCOMPort.Visible = false;
            groupUserPreferences.Visible = false;
            groupDeviceFilter.Visible = false;
            groupAPIServers.Visible = false;
            groupLocationCalibration.Visible = false;
            groupConfigSummary.Visible = false;
            groupBLELocalBond.Visible = false;

            switch (curStage)
            {
                case ConfigStage.COMSelect:
                    groupCOMPort.Visible = true;
                    break;
                case ConfigStage.UserPreferences:
                    groupUserPreferences.Visible = true;
                    break;
                case ConfigStage.DeviceFilter:
                    groupDeviceFilter.Visible = true;
                    break;
                case ConfigStage.APIServers:
                    groupAPIServers.Visible = true;
                    break;
                case ConfigStage.LocationCalib:
                    groupLocationCalibration.Visible = true;
                    break;
                case ConfigStage.BondingStage:
                    groupBLELocalBond.Visible = true;
                    break;
                case  ConfigStage.ConfigSummary:
                    groupConfigSummary.Visible = true;
                    break;
            }
        }



        private void refreshCOMList()
        {
            comboCOM.Items.Clear();

            // Populate with serial ports only if a bluegiga bluetooth dongle
            string[] serialPorts = System.IO.Ports.SerialPort.GetPortNames();
            using (var searcher = new ManagementObjectSearcher("SELECT * FROM WIN32_SerialPort"))
            {
                var ports = searcher.Get().Cast<ManagementBaseObject>().ToList();
                var tList = (from n in serialPorts join p in ports on n equals p["DeviceID"].ToString() select p["Caption"]).ToList();

                foreach (string s in tList)
                {
                    if (s.Contains("Bluegiga Bluetooth Low Energy"))
                    {
                        string[] splitStr = s.Split('(', ')');
                        foreach (string sx in splitStr)
                        {
                            if (sx.Contains("COM"))
                            {                                      
                                comboCOM.Items.Add(sx);
                                break;
                            }
                        }
                    }
                }
            }

            if (comboCOM.Items.Count > 0)
            {
                comboCOM.SelectedIndex = 0;
                butTest.Enabled = true;
            }

            StageCOM comData = getCOMData();
            labelCOMStatus.Text = comData.GetCOMStatus();
        }

        private void butRefreshCOM_Click(object sender, EventArgs e)
        {
            butRefreshCOM.Enabled = false;
            refreshCOMList();
            if (curStageData.GetType() == typeof(StageCOM))
            {
                StageCOM stage = (StageCOM)(curStageData);
                for (int i = 0; i < comboCOM.Items.Count; ++i)
                {
                    if (comboCOM.Items[i].ToString() == stage.COMPort)
                    {
                        comboCOM.SelectedIndex = i;
                        break;
                    }
                }
            }
            butRefreshCOM.Enabled = true;
        }

        private void groupBox2_Enter(object sender, EventArgs e)
        {
            
        }

        private void butNext_Click(object sender, EventArgs e)
        {
            if (curStage == ConfigStage.LocationCalib)
            {
                using (new GracefulMutex(ref isLocCalibCompleteMutex))
                {                    
                    if (!isLocCalibComplete)
                    {
                        if (DialogResult.No == MessageBox.Show(this, "Are you sure you want to skip?", "Location Calibration", MessageBoxButtons.YesNo, MessageBoxIcon.Question))
                            return;
                    }
                }
            }

            if (curStageData.isLast)
            {
                isSetupComplete = true;
                butSaveSettings_Click(sender, e);
                if (MiTokenBLE != null)
                {
                    if (conn != null)
                    {
                        conn.Disconnect();
                    }
                }
                this.Close();
            }

            if (curStageData.next(ref curStageData))
            {
                curStageData.setAsActive();
                stageRefresh();
                setToStage();
            }
        }

        private void butBack_Click(object sender, EventArgs e)
        {
            if (curStageData.prev(ref curStageData))
            {
                curStageData.setAsActive();
                stageRefresh();
                setToStage();
            }
        }

        


        

        StageCOM getCOMData()
        {
            return (StageCOM)rootStageData.GetStageOfType(typeof(StageCOM));
        }
        StageDeviceFilter getDeviceFilterData()
        {
            return (StageDeviceFilter)rootStageData.GetStageOfType(typeof(StageDeviceFilter));
        }
        StageAPIServers getAPIServersData()
        {
            return (StageAPIServers)rootStageData.GetStageOfType(typeof(StageAPIServers));
        }
        StageLocationCalib getLocationCalibData()
        {
            return (StageLocationCalib)rootStageData.GetStageOfType(typeof(StageLocationCalib));
        }
        StageConfigSummary getSummaryData()
        {
            return (StageConfigSummary)rootStageData.GetStageOfType(typeof(StageConfigSummary));
        }
        StageUserPreferences getUserPreferencesData()
        {
            return (StageUserPreferences)rootStageData.GetStageOfType(typeof(StageUserPreferences));
        }


        private void comboCOM_SelectedIndexChanged(object sender, EventArgs e)
        {
            StageCOM comData = getCOMData();
            comData.tempPort = comboCOM.SelectedItem.ToString();
            comData.isCOMPortTested = false;
            labelCOMStatus.Text = comData.GetCOMStatus();
            butTest.Enabled = true;
        }

        private void butTest_Click(object sender, EventArgs e)
        {
            this.Enabled = false;            
            using (new GracefulMutex(ref testingComMutex))
            {
                testingCOM = true;
            }
            butTest.Text = "Testing...";

            if (MiTokenBLE != null)
            {
                if (BLE_Configuration.configDictionary.ContainsKey(MiTokenBLE))
                {
                    BLE_Configuration.configDictionary.Remove(MiTokenBLE);
                }
                MiTokenBLE.CreateNewInstance();
            }
            else
            {
                MiTokenBLE = new BLE.V2_2.MiTokenBLE();
            }

            StageCOM comData = getCOMData();
            string s = comData.tempPort;
            MiTokenBLE.Initialize(s, @"\\.\pipe\Mi-TokenBLEV2", false);

            if (!BLE_Configuration.configDictionary.ContainsKey(MiTokenBLE))
            {
                BLE_Configuration.configDictionary.Add(MiTokenBLE, this);
            }

            MiTokenBLE.NewPollResults += MiTokenBLE_NewPollResults;
            MiTokenBLE.AddPollThreadInstance();
            BLE.CommandChain CC = new BLE.CommandChain();
            CC.evt.connection.disconnected = BLE_Configuration.ccc_evt_connection_disconnect;
            CC.rsp.connection.update = BLE_Configuration.ccc_rsp_connection_update;
            CC.evt.attclient.procedure_completed = BLE_Configuration.ccc_evt_procedure_completed;
            MiTokenBLE.AppendCommandChain(CC, true);

            Thread T = new Thread(new ThreadStart(() =>
                {
                    Thread.Sleep(5000);
                    using (new GracefulMutex(ref testingComMutex))
                    {
                        if (testingCOM)
                        {
                            testingCOM = false;
                            Invoke((MethodInvoker)delegate
                            {
                                butTest.Text = "Select";
                                this.Enabled = true;

                            });
                            MessageBox.Show(this, "No BLE devices detected.\nPlease have at least one device active before selecting COM port.", "Testing COM Port", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                        }
                    }
                }));
            T.Start();
        }

        void MiTokenBLE_NewPollResults(BLE.V2_1.MiTokenBLE sender)
        {
            V2_NewPollResults((BLE.V2_2.MiTokenBLE)sender);
        }

        // Class to wrap waiting and releasing mutex
        class GracefulMutex : IDisposable
        {
            public GracefulMutex(ref Mutex m)
            {
                m_refMutex = m;
                if (m_refMutex == null)
                {
                    throw new ArgumentNullException();
                }
                else
                {
                    m_refMutex.WaitOne();
                }
            }

            public void Dispose()
            {
                if (m_refMutex == null)
                {
                    throw new ArgumentNullException();
                }
                else
                {
                    m_refMutex.ReleaseMutex();
                }
            }

            private Mutex m_refMutex;
        }

        bool testingCOM = false;
        static Mutex testingComMutex = new Mutex();

        void V2_NewPollResults(BLE.V2_2.MiTokenBLE sender)
        {
            if(InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { V2_NewPollResults(sender); });

                return;
            }

            using (new GracefulMutex(ref testingComMutex))
            {
                if (testingCOM)
                {
                    int expectedPoll = sender.CurrentPollID;
                    int devicesFound = sender.GetDeviceFoundCount();
                    for (int i = 0; i < devicesFound; ++i)
                    {
                        BLE.Core.Structures.DeviceInfo devInfo = new BLE.Core.Structures.DeviceInfo();
                        sender.GetDeviceInfo(i, ref devInfo);

                        if (devInfo.lastPollSeen == expectedPoll)
                        {
                            butTest.Text = "Select";                            
                            StageCOM comData = getCOMData();
                            comData.COMPort = comData.tempPort;
                            comData.isCOMPortTested = true;
                            labelCOMStatus.Text = comData.GetCOMStatus();
                            stageRefresh();
                            testingCOM = false;
                            butTest.Enabled = false;
                            this.Enabled = true;
                        }
                    }
                }
            }

            {
                if ((lstDevices.Items.Count == 1) && (lstDevices.SelectedIndex == 0))
                {
                    if (lstDevices.SelectedItem.ToString() == scanningTxt)
                        return;
                }

                BLE.Core.Structures.DeviceInfo devInfo = new BLE.Core.Structures.DeviceInfo(), devInfo2 = new BLE.Core.Structures.DeviceInfo();
                BLE.V2_1.Filter filter = sender.CreateFilter();
                byte[] devBytes;
                string devString;
                StageDeviceFilter devFilter = getDeviceFilterData();
                if (devFilter.DeviceName != "")
                {
                    filter.AddFilter((char)9, Encoding.ASCII.GetBytes(devFilter.DeviceName));
                }
                filter.FinalizeFilter();
                var searcher = filter.createNewSearcher();
                string deviceItem = (lstDevices.SelectedIndex >= 0 ? lstDevices.SelectedItem.ToString() : " <");
                deviceItem = deviceItem.Substring(0, deviceItem.IndexOf(' '));
                lstDevices.Items.Clear();
                int newDevItemIndex = -1;
                int devFoundCount = sender.GetDeviceFoundCount();
                while (searcher.GetNextNode(ref devInfo))
                {
                    devBytes = null;
                    devString = "";

                    for (int i = 0; i < devFoundCount; ++i)
                    {
                        sender.GetDeviceInfo(i, ref devInfo2);
                        if (devInfo2.macString() == devInfo.macString())
                        {
                            sender.GetDeviceMetaInfo(i, 9, out devBytes);
                            if ((devBytes != null) && (devBytes.Length > 0))
                            {
                                devString = Encoding.ASCII.GetString(devBytes);
                            }
                        }
                    }
                    string s = devInfo.macString() + " <Signal:" + ConvertRssidBmToSignalQuality((sbyte)devInfo.RSSI).ToString() + "%" + "> " + (String.IsNullOrEmpty(devString) ? "No Name" : String.Format("{0}", devString));
                    lstDevices.Items.Add(s);
                    if (s.Substring(0, s.IndexOf(' ')) == deviceItem)
                    {
                        newDevItemIndex = lstDevices.Items.Count - 1;
                    }
                }

                lstDevices.SelectedIndex = newDevItemIndex;
                searcher.CloseFilter();

                if (lstDevices.Items.Count == 0)
                {
                    lstDevices.Items.Add(scanningTxt);
                }
            }

            return;
            if(DPC != null)
            {
                int miniPollSpeed = 100;
                int miniPollID = 0;
                int miniPollCount = 0;

                miniPollCount = DPC.results.pollSleepTime / miniPollSpeed;
                while (miniPollID < miniPollCount)
                {
                    conn.PollRSSI();
                    Thread.Sleep(miniPollSpeed);
                    miniPollID++;
                }


                byte rssi = 0;
                conn.GetRssi(out rssi);

                if (rssi == 0) //device was not seen
                {
                    rssi = 0;
                    DPC.results.setNextResult = 0;
                    if (testingRSSILevels)
                    {
                        pollSinceLastSeen++;
                    }
                }
                else //device was seen
                {
                    if (InvokeRequired)
                    {
                        Invoke((MethodInvoker)delegate
                        {
                            labLatestRSSI.Text = "Latest Signal: " + ConvertRssidBmToSignalQuality(rssi).ToString() + "%";
                        });
                    }
                    else
                    {
                        labLatestRSSI.Text = "Latest Signal: " + ConvertRssidBmToSignalQuality(rssi).ToString() + "%";
                    }

                    DPC.results.setNextResult = (sbyte)rssi;
                    if (testingRSSILevels)
                    {
                        pollSinceLastSeen = 0;
                        if (rssi > DPC.results.lockRSSI)
                        {
                            pollWithInvalidRSSI = 0;
                        }
                        else
                        {
                            pollWithInvalidRSSI++;
                        }
                    }
                }


                if (testingRSSILevels)
                {
                    if (pollSinceLastSeen > DPC.results.max0Polls)
                    {
                        pollSinceLastSeen = 0;
                        pollWithInvalidRSSI = 0;
                        DPC.results.min0Polls = DPC.results.max0Polls + 1;
                        DPC.results.resetCurrentResults();
                    }
                    else if (pollWithInvalidRSSI > DPC.results.max0Polls)
                    {
                        pollSinceLastSeen = 0;
                        pollWithInvalidRSSI = 0;
                        DPC.results.maxLockRSSI = (sbyte)(DPC.results.lockRSSI - 1);
                        DPC.results.resetCurrentResults();
                    }
                }
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            this.Enabled = false;
            StageDeviceFilter devFilter = getDeviceFilterData();
            devFilter.DeviceName = txtDeviceName.Text;
            this.Enabled = true;
        }

        private bool validateIP(string serverIP)
        {
            string requestURL = @"HTTPS://" + serverIP + @"/mi-token/api/authentication/v4/RawHttpWithSSL/AnonGenerateBLEHash";

            WebRequest req;
            try
            {
                req = WebRequest.Create(requestURL);
            }
            catch (Exception ex)
            {
                MessageBox.Show(this, ex.Message, "Exception", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return false;
            }

            req.Proxy = null;
            req.Method = "GET";
            req.Timeout = 3000;

            try
            {
                WebResponse resp = req.GetResponse();
                return true; //we got a response from the server
            }
            catch (Exception ex)
            {
                return false; //we didn't get a response from the server
            }
        }

        private void butValidateMiTokenAPIServer_Click(object sender, EventArgs e)
        {
            this.Enabled = false;
            if (lstAPIServers.Items.Count > 0)
            {
                if(MessageBox.Show(string.Format("The current version of the Mi-Token BLE Login only allows 1 Mi-Token API Server, continuing will remove the current server of {0}\r\nRemove {0} and attempt to add {1}?", lstAPIServers.Items[0], txtMiTokenAPIServer.Text), "Validate and Add", MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == System.Windows.Forms.DialogResult.No)
                {
                    this.Enabled = true;
                    return;
                }
            }

            bool allowServer = false;
            string temp = butValidateMiTokenAPIServer.Text;
            butValidateMiTokenAPIServer.Text = "Validating...";
            Application.DoEvents();
            if (!(allowServer = validateIP(txtMiTokenAPIServer.Text)))
            {
                allowServer = (MessageBox.Show(string.Format("The server {0} did not respond to the validation request. Would you still like to add this server?", txtMiTokenAPIServer.Text), "Warning", MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == System.Windows.Forms.DialogResult.Yes);
            }

            butValidateMiTokenAPIServer.Text = temp;
            Application.DoEvents();


            if (allowServer)
            {
                StageAPIServers apiServers = getAPIServersData();
                apiServers.APIServers.Clear();
                apiServers.APIServers.Add(txtMiTokenAPIServer.Text);
                lstAPIServers.Items.Clear();

                foreach (string s in apiServers.APIServers)
                {
                    lstAPIServers.Items.Add(s);
                }

                txtMiTokenAPIServer.Text = "";
                stageRefresh();
            }
            this.Enabled = true;
        }

        Thread RSSISetupThread = null;
        Thread pollingThread = null;

        enum RSSI_Stage
        {
            Connecting,
            SettingUp,
            Calibrating,
            Login,
            Lock,
            NoLock,
            Completed,
            Terminated

        };

        private void updateNewRSSIStage(RSSI_Stage stage)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { updateNewRSSIStage(stage); });
                return;
            }

            progStageProgress.Value = 0;

            switch (stage)
            {
                case RSSI_Stage.Calibrating:
                    txtStageID.Text = "Calibrating Poll Speed";
                    txtAction.Text = "Place the device close to the computer then either press the button on the device or the 'Continue' button below";
                    buttonContinue.Enabled = true;
                    break;
                case RSSI_Stage.Completed:
                    txtStageID.Text = "Completed";
                    txtAction.Text = "Press 'Next' to complete calibration";
                    buttonContinue.Enabled = false;
                    using (new GracefulMutex(ref isLocCalibCompleteMutex))
                    {
                        isLocCalibComplete = true;
                    }
                    break;
                case RSSI_Stage.Connecting:
                    txtStageID.Text = "Connecting";
                    txtAction.Text = "Please wait...";
                    buttonContinue.Enabled = true;
                    break;
                case RSSI_Stage.Lock:
                    txtStageID.Text = "Lock Location";
                    txtAction.Text = "Place the device in a location you would like it to lock from and then press 'Continue'";
                    buttonContinue.Enabled = true;
                    break;
                case RSSI_Stage.Login:
                    txtStageID.Text = "Login Location";
                    txtAction.Text = "Place the device in a location you would like it to login from and then press 'Continue'";
                    buttonContinue.Enabled = true;
                    break;
                case RSSI_Stage.NoLock:
                    txtStageID.Text = "Session Location";
                    txtAction.Text = "Place the device in a location during a normal session when you are logged in and then press 'Continue'";
                    buttonContinue.Enabled = true;
                    break;
                case RSSI_Stage.SettingUp:
                    txtStageID.Text = "Setting Up Device";
                    txtAction.Text = "Please wait...";
                    buttonContinue.Enabled = true;
                    break;
            }
        }

        private void showWaitMessage()
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { showWaitMessage(); });
                return;
            }

            txtAction.Text = "Please wait...";
        }

        byte[] addressFromString(string str)
        {
            string[] substr = str.Split(':');
            if (substr.Length != 6)
            {
                throw new ArgumentException("Expected a MAC address, got something different", "str");
            }

            byte[] bindata = new byte[6];
            for (int i = 0; i < 6; ++i)
            {
                bindata[i] = Convert.ToByte(substr[i], 16);
            }

            return bindata;
        }

        RSSI_Stage curRSSIStage = RSSI_Stage.Connecting;
        BLE.V2_2.Device conn;

        Mutex isLocCalibCompleteMutex = new Mutex();
        Boolean isLocCalibComplete = false;
        private void StartLocationCalibration()
        {
            using (new GracefulMutex(ref isLocCalibCompleteMutex))
            {
                isLocCalibComplete = false;
            }

            StageDeviceFilter deviceFilterData = getDeviceFilterData();
            if (deviceFilterData.DeviceMacToMonitor == "")
                return;

            StageLocationCalib locData = getLocationCalibData();            

            if (RSSISetupThread != null)
            {
                RSSISetupThread.Abort();
            }

            RSSISetupThread = new Thread(new ThreadStart( () =>
            {

                RSSI_Stage curRSSIStage = RSSI_Stage.Connecting;
                updateNewRSSIStage(curRSSIStage);

                int devCount = MiTokenBLE.GetDeviceFoundCount();

                BLE.Core.Structures.DeviceInfo devInfo = new BLE.Core.Structures.DeviceInfo();
                bool found = false;
                for (int i = 0; (i < devCount) && (!found); ++i)
                {
                    MiTokenBLE.GetDeviceInfo(i, ref devInfo);
                    if (devInfo.macString() == deviceFilterData.DeviceMacToMonitor)
                    {
                        found = true;
                    }
                }

                if (found)
                {
                    disconnectionEvent.Reset();
                    conn = MiTokenBLE.connectToDevice(devInfo);
                }
                else
                {
                    Invoke((MethodInvoker)delegate
                    {
                        MessageBox.Show(this, "Could not find the device to connect to", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    });
                    return;
                }


                if (conn.SyncWaitForConnection(1000) == BLE.Core.CONN_RET.BLE_CONN_ERR_SYNC_TIMEOUT)
                {
                    Invoke((MethodInvoker)delegate
                    {
                        txtAction.Text = "Connection is taking longer than normal. Please ensure that the device is on.";
                    });

                    conn.SyncWaitForConnection(BLE.Core.CONN_TIMEOUT_INFINITE);
                }

#if false // Disable registering for push button notifications
                string curStatus;
                while (!conn.SuccessfulReturn(conn.ScanServicesInRange(MiToken_BLE_UUIDs.BLE_UUID_SERVICE_PUSH_BUTTON, 0x20), out curStatus))
                {
                    Thread.Sleep(100);
                }
#endif
                curRSSIStage = RSSI_Stage.SettingUp;
                updateNewRSSIStage(curRSSIStage);
#if false // Disable registering for push button notifications
                byte[] notifyData = new byte[] { 0x01, 0x00 };
                while (!conn.SuccessfulReturn(conn.SetAttribute(MiToken_BLE_UUIDs.BLE_UUID_ATTRIBUTE_PUSH_BUTTON_CHARACTERISTIC_CONFIG, notifyData), out curStatus))
                {
                    Thread.Sleep(100);
                }
#endif

                //TODO : Fix Button Pressed Callback!

                DPCDeviceMac = deviceFilterData.DeviceMacToMonitor;
                DPC = new DevicePollConfig(conn);

                curRSSIStage = RSSI_Stage.Login;
                updateNewRSSIStage(curRSSIStage);
                if (!waitForEvent())
                {
                    //we disconnected
                    return;
                }
                DPC.results.setResults(DevicePollConfig.SampleResults.ResultType.Unlock);
                bool runPoll = true;

                if (pollingThread != null)
                {
                    pollingThread.Abort();
                }
                pollingThread = new Thread(new ThreadStart(() =>
                    {
                        while (runPoll)
                        {
                            int miniPollSpeed = 100;
                            int miniPollID = 0;
                            int miniPollCount = DPC.results.pollSleepTime / miniPollSpeed;
                            
                            while (miniPollID < miniPollCount)
                            {
                                conn.PollRSSI();
                                Thread.Sleep(miniPollSpeed);
                                miniPollID++;
                            }

                            byte rssi = 0;
                            conn.GetRssi(out rssi);
                            if (Program.debugMode)
                            {
                                Invoke((MethodInvoker)delegate
                                {
                                    butDebug.Font = new Font(butDebug.Font.FontFamily, 16);
                                    butDebug.Text = rssi.ToString();
                                });
                            }


                            if (rssi == 0)
                            {
                                rssi = 0;
                                DPC.results.setNextResult = 0;
                                if (testingRSSILevels)
                                {
                                    pollSinceLastSeen++;
                                }
                            }
                            else
                            {
                                Invoke((MethodInvoker)delegate
                                {
                                    labLatestRSSI.Text = "Latest Signal: " + ConvertRssidBmToSignalQuality(rssi).ToString() + "%";
                                });
                            }
                            DPC.results.setNextResult = (sbyte)rssi;
                            if (testingRSSILevels)
                            {
                                pollSinceLastSeen = 0;
                                if (rssi > DPC.results.lockRSSI)
                                {
                                    pollWithInvalidRSSI = 0;
                                }
                                else
                                {
                                    pollWithInvalidRSSI++;
                                }
                            }
                        }

                        if (testingRSSILevels)
                        {
                            if (pollSinceLastSeen > DPC.results.max0Polls)
                            {
                                pollSinceLastSeen = 0;
                                pollWithInvalidRSSI = 0;
                                DPC.results.min0Polls = DPC.results.max0Polls + 1;
                                DPC.results.resetCurrentResults();
                            }
                            else if(pollWithInvalidRSSI > DPC.results.max0Polls)
                            {
                                pollSinceLastSeen = 0;
                                pollWithInvalidRSSI = 0;
                                DPC.results.maxLockRSSI = (sbyte)(DPC.results.lockRSSI - 1);
                                DPC.results.resetCurrentResults();
                            }
                        }
                    }));

                pollingThread.Start();

                while ((curRSSIStage != RSSI_Stage.Completed) && (curRSSIStage != RSSI_Stage.Terminated))
                {
                    showWaitMessage();
                    if (InvokeRequired)
                    {
                        Invoke((MethodInvoker)delegate
                        {
                            progStageProgress.Value = DPC.results.progressPercent;
                            labUnlockRSSI.Text = "Login Signal: " + ConvertRssidBmToSignalQuality(DPC.results.unlockRSSI).ToString() + "%";
                            labLockRSSI.Text = "Lock Signal: " + ConvertRssidBmToSignalQuality(DPC.results.lockRSSI).ToString() + "%";
                        });
                    }

                    if (disconnectionEvent.WaitOne(0))
                    {
                        //we lost the connection
                        switch (curRSSIStage)
                        {
                            case RSSI_Stage.Login:
                                Invoke((MethodInvoker)delegate
                                {
                                    MessageBox.Show(this, "Device was disconnected during the Login Calibration. This is normally caused by either pressing the power button on the device, or the device being too far away from the machine.\r\n\r\nYou will need to restart calibration.", "Error : Disconnect during Login", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                                    conn.Disconnect();
                                    butBack_Click(null, null);
                                    butNext_Click(null, null);
                                });
                                return;

                            case RSSI_Stage.Lock:
                                bool close = false;
                                Invoke((MethodInvoker)delegate
                                {
                                    if (MessageBox.Show("Device was disconnected during the Lock Calibration. This is normally caused by either pressing the power button on the device, or the device being too far away from the machine.\r\n\r\nNote: This result is still valid, Mi-Token can be configured to lock the machine only when the device becomes disconnected. Would you like to use this configuration?\r\nYes : Lock only on device disconnection\r\nNo : Re-run the calibration tool from the start.", "Caution : Disconnection during Lock", MessageBoxButtons.YesNo, MessageBoxIcon.Information) == System.Windows.Forms.DialogResult.Yes)
                                    {
                                        //YES
                                        DPC.lockOnlyOnDisconnect = true;
                                        //no need to validate with this setup
                                        curRSSIStage = RSSI_Stage.Completed;
                                    }
                                    else
                                    {
                                        butBack_Click(null, null);
                                        butNext_Click(null, null);
                                        //NO
                                    }
                                });
                                if(close)
                                {
                                    return;
                                }
                                updateNewRSSIStage(curRSSIStage);
                                break;
                            case RSSI_Stage.NoLock:
                                Invoke((MethodInvoker)delegate
                                {
                                    MessageBox.Show(this, "Device was disconnected during the Verification Calibration. This is normally caused by either pressing the power button on the device, or the device being too far away from the machine.\r\n\r\nYou will need to restart calibration.", "Error : Disconnection during Verification", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                                    butBack_Click(null, null);
                                    butNext_Click(null, null);
                                });
                                return;
                        }
                    }

                    switch (curRSSIStage)
                    {
                        case RSSI_Stage.Calibrating:
                            if (DPC.results.currentlyTesting == DevicePollConfig.SampleResults.ResultType.None)
                            {
                                curRSSIStage = RSSI_Stage.Login;
                                updateNewRSSIStage(curRSSIStage);
                                if (!waitForEvent())
                                {
                                    int x = 0;
                                }
                            }
                            break;
                        case RSSI_Stage.Login:
                            if (DPC.results.currentlyTesting == DevicePollConfig.SampleResults.ResultType.None)
                            {
                                if (DPC.results.RecommendSlowerSampleRate())
                                {
                                    DPC.results.pollSleepTime += 100;
                                    DPC.results.flushAllResults();
                                    curRSSIStage = RSSI_Stage.Login;
                                    updateNewRSSIStage(curRSSIStage);
                                    if (!waitForEvent())
                                    {
                                        int x = 1;
                                    }
                                    DPC.results.setResults(DevicePollConfig.SampleResults.ResultType.Unlock);
                                }
                                else
                                {
                                    curRSSIStage = RSSI_Stage.Lock;
                                    updateNewRSSIStage(curRSSIStage);
                                    if (!waitForEvent())
                                    {
                                        int x = 2;
                                    }
                                    DPC.results.setResults(DevicePollConfig.SampleResults.ResultType.Lock);
                                }
                            }
                            break;
                        case RSSI_Stage.Lock:
                            if (DPC.results.currentlyTesting == DevicePollConfig.SampleResults.ResultType.None)
                            {
                                if (DPC.results.RecommendSlowerSampleRate())
                                {
                                    DPC.results.pollSleepTime += 100;
                                    DPC.results.flushAllResults();
                                    curRSSIStage = RSSI_Stage.Login;
                                    updateNewRSSIStage(curRSSIStage);
                                    if (!waitForEvent())
                                    {
                                        int x = 3;
                                    }
                                    DPC.results.setResults(DevicePollConfig.SampleResults.ResultType.Unlock);
                                }
                                else
                                {
                                    curRSSIStage = RSSI_Stage.NoLock;
                                    updateNewRSSIStage(curRSSIStage);
                                    if (!waitForEvent())
                                    {
                                        int x = 4;
                                    }
                                    DPC.results.setResults(DevicePollConfig.SampleResults.ResultType.Test);
                                    testingRSSILevels = true;
                                }
                            }
                            break;
                        case RSSI_Stage.NoLock:
                            if (DPC.results.currentlyTesting == DevicePollConfig.SampleResults.ResultType.None)
                            {
                                curRSSIStage = RSSI_Stage.Completed;
                                updateNewRSSIStage(curRSSIStage);
                            }
                            break;
                    }
                }

                runPoll = false;


                StageLocationCalib lc = getLocationCalibData();
                lc.unlockRSSI = (byte)DPC.results.unlockRSSI;
                lc.lockRSSI = (byte)DPC.lockRSSI;
                lc.setup = true;

                conn.Disconnect();

                stageRefresh();

            }));

            RSSISetupThread.Start();
        }

        AutoResetEvent waitForConfirmation = new AutoResetEvent(false);

        private bool waitForEvent()
        {
            waitForConfirmation.WaitOne(0, false);

            //do a non-blocking wait just in case the button was already pressed and the event already signalled
            WaitHandle[] handles = new WaitHandle[] { waitForConfirmation };
            if (WaitHandle.WaitAny(handles) == 1)
            {
                return false;
            }

            return true;
        }

        void conn_ButtonPressed(int requestID, int buttonValue)
        {
            if ((buttonValue & 1) != 0)
            {
                //pressed power button
                curRSSIStage = RSSI_Stage.Terminated;
            }

            waitForConfirmation.Set();
        }

        private void buttonContinue_Click(object sender, EventArgs e)
        {
            waitForConfirmation.Set();
        }

        private void BLE_Configuration_Load(object sender, EventArgs e)
        {
            // Update dialog text with name and version
            Assembly assembly = Assembly.GetExecutingAssembly();
            this.Text = "Configuration Tool V" + FileVersionInfo.GetVersionInfo(assembly.Location).FileVersion;

            // Stop Watcher Service
            var sc = getWatcherService();
            try
            {
                // Only attempt to stop service if able to
                if (sc != null)
                {
                    if (sc.CanStop)
                    {
                        sc.Stop();
                    }
                }
            }
            catch
            {
                Debug.WriteLine(String.Format("{0}\tBLE_Configuration_Load:Could not stop Watcher service", DateTime.Now));
            }

            if (Program.debugMode)
            {
                butDebug.Visible = true;
            }
        }

        public enum ServiceState
        {
            Uninstalled,
            InstalledAndStopped,
            InstalledAndRunnig,
            InstalledAndUnknown,
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
                MessageBox.Show(this, ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return null;
            }            
        }

        public ServiceState getServiceState()
        {
            System.ServiceProcess.ServiceController sc = getWatcherService();
            if (sc == null)
            {
                return ServiceState.Uninstalled;
            }

            try
            {
                switch (sc.Status)
                {
                    case System.ServiceProcess.ServiceControllerStatus.ContinuePending:
                    case System.ServiceProcess.ServiceControllerStatus.Paused:
                    case System.ServiceProcess.ServiceControllerStatus.PausePending:
                    case System.ServiceProcess.ServiceControllerStatus.Running:
                    case System.ServiceProcess.ServiceControllerStatus.StartPending:
                    case System.ServiceProcess.ServiceControllerStatus.StopPending:
                        return ServiceState.InstalledAndRunnig;
                    case System.ServiceProcess.ServiceControllerStatus.Stopped:
                        return ServiceState.InstalledAndStopped;
                    default:
                        return ServiceState.InstalledAndUnknown;
                }
            }

            catch
            {
                return ServiceState.Uninstalled;
            }
        }

        private void butSaveSettings_Click(object sender, EventArgs e)
        {
            this.Enabled = false;
            using (RegistryKey regKey = getRegKey(true))
            {
                if (regKey != null)
                {
                    getCOMData().SaveToRegistry(regKey);
                }
            }
            this.Enabled = true;
        }


        private void doCommandLine(string processPath, string args)
        {
            var process = new System.Diagnostics.Process();
            process.StartInfo.Arguments = args;
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.FileName = processPath;
            process.StartInfo.RedirectStandardOutput = true;
            process.StartInfo.RedirectStandardError = true;
            process.Start();

            string output = process.StandardOutput.ReadToEnd();
            string error = process.StandardError.ReadToEnd();
            process.WaitForExit();

            //MessageBox.Show("Debug Output:\r\nOutput : \r\n" + output + "\r\nError : \r\n" + error);
        }

        private void uninstallService()
        {            
            doCommandLine(System.Runtime.InteropServices.RuntimeEnvironment.GetRuntimeDirectory() + "InstallUtil.exe", string.Format("-u \"{0}/BLEWatcher.exe\"", Application.StartupPath));
        }

        private void killAllLockers()
        {
            doCommandLine("taskkill.exe", @"/f /im BLELocker.exe");
        }

        private void installService()
        {
            //first copy over the file (we will have the service stored as a different file on the filesystem so that installing is easier)
            string fromServiceFile = @"\Service.dat";
            string toServiceFile = @"\BLEWatcher.exe";
            string fromLockerFile = @"\BLELockout.dat";
            string toLockerFile = @"\BLELockout.exe";

            bool cannotInstallService = false;
            bool cannotUpdateLockout = false;
            fromServiceFile = Application.StartupPath + fromServiceFile;
            toServiceFile = Application.StartupPath + toServiceFile;
            fromLockerFile = Application.StartupPath + fromLockerFile;
            toLockerFile = Application.StartupPath + toLockerFile;
            if (System.IO.File.Exists(toServiceFile))
            {
                try
                {
                    System.IO.File.Delete(toServiceFile);
                }
                catch (Exception ex)
                {
                    MessageBox.Show(this, "Failed to update Service File " + toServiceFile + "\r\nYou may need to restart your computer then try updating the service again", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    cannotInstallService = true;
                }
            }

            if (!cannotInstallService)
            {
                try
                {
                    System.IO.File.Copy(fromServiceFile, toServiceFile);
                }
                catch (Exception ex)
                {
                    MessageBox.Show(this, ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }

            killAllLockers();

            if (System.IO.File.Exists(toLockerFile))
            {
                try
                {
                    System.IO.File.Delete(toLockerFile);
                }
                catch (Exception ex)
                {
                    MessageBox.Show(this, "Failed to update the Locker File " + toLockerFile + "\r\nPlease ensure there are no instances of it running then try again", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    cannotUpdateLockout = true;
                }
            }

            if (!cannotUpdateLockout)
            {
                try
                {
                    System.IO.File.Copy(fromLockerFile, toLockerFile);
                }
                catch (Exception ex)
                {
                    MessageBox.Show(this, ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }

            if (cannotUpdateLockout || cannotInstallService)
            {
                return;
            }
            
            // Install service
            doCommandLine(System.Runtime.InteropServices.RuntimeEnvironment.GetRuntimeDirectory() + @"InstallUtil.exe", "\"" + toServiceFile + "\"");
        }

        private void butUpdateService_Click(object sender, EventArgs e)
        {
            this.Enabled = false;
            if (getServiceState() != ServiceState.Uninstalled)
            {
                uninstallService();
            }

            installService();

            getSummaryData().setAsActive();
            stageRefresh();
            this.Enabled = true;
        }

        private void butServiceStartStop_Click(object sender, EventArgs e)
        {
            System.ServiceProcess.ServiceController sc = getWatcherService();
            ServiceState ss = getServiceState();
            switch (ss)
            {
                case ServiceState.InstalledAndStopped:
                case ServiceState.InstalledAndUnknown:
                    sc.Start();
                    break;
                case ServiceState.InstalledAndRunnig:
                    sc.Stop();
                    break;
                case ServiceState.Uninstalled:
                    //should never be possible
                    break;
            }

            getSummaryData().setAsActive();
            stageRefresh();
        }

        private void groupCOMPort_Enter(object sender, EventArgs e)
        {

        }

        private void butDebug_Click(object sender, EventArgs e)
        {
            if (Program.debugMode)
            {
                BLEWatcherStateMachine.BLEDebug debugForm = new BLEWatcherStateMachine.BLEDebug(MiTokenBLE);
                debugForm.Show();
            }
        }

        private void butBondToDevice_Click(object sender, EventArgs e)
        {
            StageDeviceFilter deviceFilterData = getDeviceFilterData();
            if (deviceFilterData.DeviceMacToMonitor == "")
            {
                // This should never happen
                MessageBox.Show(this, "Device not selected!", "Create Local Bond", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            if (txtBondUser.Text == "")
            {
                MessageBox.Show(this, "Local Bond User cannot be empty!", "Create Local Bond", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

            string devMac = deviceFilterData.DeviceMacToMonitor;
            if (MessageBox.Show(string.Format("Are you sure you would like to create a local bond with the following device\r\n\t{0}\r\n\r\nPlease note : Each device can have at most 1 local bond and each machine can have at most 1 bound token. Also you cannot remove local bonds without re-provisioning the device.", devMac), "Confirmation", MessageBoxButtons.OKCancel, MessageBoxIcon.Question) != System.Windows.Forms.DialogResult.OK)
            {
                return;
            }

            String originalButText = butBondToDevice.Text;
            butBondToDevice.Text = "Bonding...";
            this.Enabled = false;
            Thread ProcessingThread = null;
            ProcessingThread = new Thread(new ThreadStart(() =>
                {
                    try
                    {
                        BLE.Core.Structures.DeviceInfo devInfo = new BLE.Core.Structures.DeviceInfo();
                        int count = MiTokenBLE.GetDeviceFoundCount();
                        for (int i = 0; i < count; ++i)
                        {
                            MiTokenBLE.GetDeviceInfo(i, ref devInfo);
                            if (devInfo.macString().ToLower().Equals(devMac.ToLower()))
                            {
                                BLE.V2_2.Device conn = MiTokenBLE.connectToDevice(devInfo);
                                if (BLE.Core.CONN_RET.BLE_CONN_ERR_SYNC_TIMEOUT == conn.SyncWaitForConnection(2000))
                                {
                                    Invoke((MethodInvoker)delegate
                                    {
                                        MessageBox.Show(this, "Create local bond failed. Could not connect to device.", originalButText, MessageBoxButtons.OK, MessageBoxIcon.Error);
                                        butBondToDevice.Text = originalButText;
                                        this.Enabled = true;
                                    });
                                    return;
                                }
                                // Ensure connection is in sync
                                bool result = SyncUpdateConnectionParameters(conn, connectionUpdateResponseSemaphore, connectionUpdateResponseEvent, procedureCompletedEvent);

                                // Attempt to add bond and save to registry
                                if (result)
                                {
                                    byte[] bond = { 0 };
                                    BLE.Core.BOND_RET bondResult = conn.AddBond(out bond);
                                    conn.Disconnect();

                                    switch (bondResult)
                                    {
                                        case BLE.Core.BOND_RET.BOND_RET_SUCCESS:
                                            using (RegistryKey key = getRegKey(true))
                                            {
                                                if (key != null)
                                                {
                                                    key.SetValue("BondDeviceCID", (Int32)0);
                                                    key.SetValue("BondDeviceH", (bond));
                                                    key.SetValue("BondDeviceMac", devInfo.macAddress);
                                                }
                                                else
                                                {
                                                    result = false;
                                                    MessageBox.Show(this, "Create local bond failed. Successfully created bond but could not save to registry.", originalButText, MessageBoxButtons.OK, MessageBoxIcon.Error);
                                                }
                                            }

                                            Invoke((MethodInvoker)delegate
                                            {
                                                MessageBox.Show(this, "Local bond was created successfully", originalButText, MessageBoxButtons.OK, MessageBoxIcon.Information);
                                                butBondToDevice.Text = originalButText;
                                                this.Enabled = true;
                                            });
                                            break;
                                        case BLE.Core.BOND_RET.BOND_RET_FAILED:
                                            Invoke((MethodInvoker)delegate
                                            {
                                                MessageBox.Show(this, "Create local bond failed. Verification failed.", originalButText, MessageBoxButtons.OK, MessageBoxIcon.Error);
                                                butBondToDevice.Text = originalButText;
                                                this.Enabled = true;
                                            });
                                            break;
                                        case BLE.Core.BOND_RET.BOND_RET_ERR_TIMEOUT:
                                            Invoke((MethodInvoker)delegate
                                            {
                                                MessageBox.Show(this, "Create local bond failed. The device may have disconnected.", originalButText, MessageBoxButtons.OK, MessageBoxIcon.Error);
                                                butBondToDevice.Text = originalButText;
                                                this.Enabled = true;
                                            });
                                            break;
                                        case BLE.Core.BOND_RET.BOND_RET_ERR_PREVBOND_MAXBOND:
                                            Invoke((MethodInvoker)delegate
                                            {
                                                MessageBox.Show(this, "Create local bond failed. The device may have a local bond already set or the maximum amount of bonds has been reached. Attempt to remove all bonds.", originalButText, MessageBoxButtons.OK, MessageBoxIcon.Error);
                                                butBondToDevice.Text = originalButText;
                                                this.Enabled = true;
                                            });
                                            break;
                                        default:
                                            Invoke((MethodInvoker)delegate
                                            {
                                                MessageBox.Show(this, "Create local bond failed. Unhandled Exception.", originalButText, MessageBoxButtons.OK, MessageBoxIcon.Error);
                                                butBondToDevice.Text = originalButText;
                                                this.Enabled = true;
                                            });
                                            break;
                                    }
                                }
                                else
                                {
                                    conn.Disconnect();

                                    Invoke((MethodInvoker)delegate
                                    {
                                        MessageBox.Show(this, "Create local bond failed. Connection could not be established.", originalButText, MessageBoxButtons.OK, MessageBoxIcon.Error);
                                        butBondToDevice.Text = originalButText;
                                        this.Enabled = true;
                                    });
                                }                                
                            }
                        }
                    }
                    catch (ThreadAbortException)
                    {
                        Debug.WriteLine(String.Format("butBondToDevice_Click:Processing Thread aborted"));
                        conn.Disconnect();
                    }
                }));
            ProcessingThread.Start();
        }

        private void buttonRemoveBonds_Click(object sender, EventArgs e)
        {
            StageDeviceFilter deviceFilterData = getDeviceFilterData();
            if (deviceFilterData.DeviceMacToMonitor == "")
            {
                // This should never happen
                MessageBox.Show(this, "Device not selected!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            string devMac = deviceFilterData.DeviceMacToMonitor;
            if (MessageBox.Show(string.Format("Are you sure you would like to  remove all bonds from this device\r\n\t{0}\r\n\r\nPlease note : You will need to reset the Mi-Token API Bond back with the Provisioning tool.", devMac), "Confirmation", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning) != System.Windows.Forms.DialogResult.OK)
            {
                return;
            }

            String originalButText = buttonRemoveBonds.Text;
            buttonRemoveBonds.Text = "Removing Bond...";
            this.Enabled = false;
            Thread ProcessingThread = null;
            ProcessingThread = new Thread(new ThreadStart(() =>
                {
                    try
                    {
                        BLE.Core.Structures.DeviceInfo devInfo = new BLE.Core.Structures.DeviceInfo();
                        int count = MiTokenBLE.GetDeviceFoundCount();
                        for (int i = 0; i < count; ++i)
                        {
                            MiTokenBLE.GetDeviceInfo(i, ref devInfo);
                            if (devInfo.macString().ToLower().Equals(devMac.ToLower()))
                            {
                                BLE.V2_2.Device conn = MiTokenBLE.connectToDevice(devInfo);
                                if (BLE.Core.CONN_RET.BLE_CONN_ERR_SYNC_TIMEOUT == conn.SyncWaitForConnection(2000))
                                {
                                    Invoke((MethodInvoker)delegate
                                    {
                                        MessageBox.Show(this, "Removing bonds failed. Could not connect to device.", originalButText, MessageBoxButtons.OK, MessageBoxIcon.Warning);
                                        buttonRemoveBonds.Text = originalButText;
                                        this.Enabled = true;
                                    });
                                    return;
                                }
                                // Ensure connection is in sync
                                bool result = SyncUpdateConnectionParameters(conn, connectionUpdateResponseSemaphore, connectionUpdateResponseEvent, procedureCompletedEvent);

                                // Attempt to remove all bonds
                                if (result)
                                {
                                    result = conn.RemoveAllBonds();
                                    if (result)
                                    {
                                        // Remove from registry only if valid
                                        using (RegistryKey key = getRegKey(true))
                                        {
                                            if (key != null)
                                            {
                                                key.DeleteValue("BondDeviceCID", false);
                                                key.DeleteValue("BondDeviceH", false);
                                                key.DeleteValue("BondDeviceMac", false);
                                            }
                                        }
                                    }
                                }

                                conn.Disconnect();

                                Invoke((MethodInvoker)delegate
                                {
                                    buttonRemoveBonds.Text = originalButText;
                                    this.Enabled = true;
                                });
                                if (result)
                                {
                                    Invoke((MethodInvoker)delegate
                                    {
                                        txtBondUser.Text = "";
                                        MessageBox.Show(this, "All bonds have been removed", originalButText, MessageBoxButtons.OK, MessageBoxIcon.Information);
                                    });                                    
                                }
                                else
                                {
                                    Invoke((MethodInvoker)delegate
                                    {
                                        MessageBox.Show(this, "Removing bonds failed. The device may have disconnected.", originalButText, MessageBoxButtons.OK, MessageBoxIcon.Error);
                                    });
                                }
                            }
                        }
                    }
                    catch (ThreadAbortException)
                    {
                        Debug.WriteLine(String.Format("buttonRemoveBonds_Click:Processing Thread aborted"));
                        conn.Disconnect();
                    }
                }));
            ProcessingThread.Start();
        }

        static private bool SyncUpdateConnectionParameters(BLE.V2_2.Device conn, SemaphoreSlim connectionUpdateResponseSemaphore, ManualResetEventSlim connectionUpdateResponseEvent, ManualResetEventSlim procedureCompletedEvent)
        {
            Debug.WriteLine(String.Format("{0}\tWaiting for residual events to complete", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt")));
            procedureCompletedEvent.Reset();
            Stopwatch sw = new Stopwatch();
            sw.Start();
            procedureCompletedEvent.Wait(2000);            
            sw.Stop();
            Debug.WriteLine(String.Format("{0}\tResidual events complete ElapsedMilliseconds={1}", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt"), sw.ElapsedMilliseconds));

            Debug.WriteLine(String.Format("{0}\tSemaphore Wait ENTER", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt")));
            // Ensure connection is in sync; Wait for response from update before continuing
            if (!connectionUpdateResponseSemaphore.Wait(1000))
            {
                return false;
            }

            Debug.WriteLine(String.Format("{0}\tSemaphore Wait EXIT", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt")));
            Debug.WriteLine(String.Format("{0}\tEvent Reset", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt")));
            connectionUpdateResponseEvent.Reset();
            Debug.WriteLine(String.Format("{0}\tSend Connection Update", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt")));
            conn.UpdateConnectionParameters(8, 16, 50, 0);
            Debug.WriteLine(String.Format("{0}\tEvent Wait ENTER", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt")));
            bool connectionUpdateResponseResult = connectionUpdateResponseEvent.Wait(1000);
            Debug.WriteLine(String.Format("{0}\tEvent Wait EXIT", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt")));
            connectionUpdateResponseSemaphore.Release();
            Debug.WriteLine(String.Format("{0}\tSemaphore Release", DateTime.Now.ToString("dd/MM/yyyy hh:mm:ss.fff tt")));
            return connectionUpdateResponseResult;
        }

        private string scanningTxt = "Scanning...";
        private void lstDevices_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (curStage != ConfigStage.DeviceFilter)
                return;

            if (lstDevices.SelectedIndex == -1)
            {
                stageRefresh();
                return;
            }

            StageDeviceFilter devFilterData = getDeviceFilterData();
            string s = lstDevices.SelectedItem.ToString();
            if (s.CompareTo(scanningTxt) == 0)
            {
                lstDevices.ClearSelected();
                return;
            }
            s = s.Substring(0, s.IndexOf(' '));
            devFilterData.DeviceMacToMonitor = s;
            labelBLELocalBondDeviceSelected.Text = s;
            stageRefresh();
        }

        private void txtBondUser_TextChanged(object sender, EventArgs e)
        {
            if (curStage != ConfigStage.BondingStage)
                return;

            if (txtBondUser.Text != "")
                butBondToDevice.Enabled = true;
            else
                butBondToDevice.Enabled = false;
        }

        private void BLE_Configuration_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (!isSetupComplete)
            {
                if (DialogResult.No == MessageBox.Show(this, "You may lose settings if you quit prematurely.\nAre you sure you want to quit?", "Quit", MessageBoxButtons.YesNo, MessageBoxIcon.Warning))
                {
                    e.Cancel = true;
                    return;
                }
            }

            if (RSSISetupThread != null)
            {
                RSSISetupThread.Abort();
            }

            if (pollingThread != null)
            {
                pollingThread.Abort();
            }

            if (MiTokenBLE != null)
            {
                if (BLE_Configuration.configDictionary.ContainsKey(MiTokenBLE))
                {
                    BLE_Configuration.configDictionary.Remove(MiTokenBLE);
                }
                MiTokenBLE.DeleteInstance();
            }

            var sc = getWatcherService();
            try
            {
                if (sc != null)
                    sc.Start();
            }
            catch
            {
                Debug.WriteLine(String.Format("{0}\tBLE_Configuration_FormClosing:Could not start Watcher service", System.DateTime.Now));
            }

            if (!isSetupComplete)
            {
                Environment.ExitCode = -2;
            }
        }

        private void lstAPIServers_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (curStage != ConfigStage.APIServers)
                return;

            if (lstAPIServers.SelectedIndex == -1)
            {
                butRemoveMiTokenAPIServer.Enabled = false;
            }
            else
            {
                butRemoveMiTokenAPIServer.Enabled = true;
            }
        }

        private void txtMiTokenAPIServer_TextChanged(object sender, EventArgs e)
        {
            if (curStage != ConfigStage.APIServers)
                return;

            if (txtMiTokenAPIServer.Text == "")
                butValidateMiTokenAPIServer.Enabled = false;
            else
                butValidateMiTokenAPIServer.Enabled = true;
        }

        private void butRemoveMiTokenAPIServer_Click(object sender, EventArgs e)
        {
            if (curStage != ConfigStage.APIServers)
                return;

            if (lstAPIServers.SelectedIndex == -1)
                return;

            StageAPIServers APIServersData = getAPIServersData();
            APIServersData.APIServers.Remove(lstAPIServers.SelectedItem.ToString());
            lstAPIServers.Items.Remove(lstAPIServers.SelectedItem);
            stageRefresh();
        }

        static public int ConvertRssidBmToSignalQuality(byte rssi)
        {
            return ConvertRssidBmToSignalQuality((sbyte)rssi);
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

        private void radioButtonUserSingle_Click(object sender, EventArgs e)
        {
            StageUserPreferences data = getUserPreferencesData();
            data.IsUserMulti = false;
        }

        private void radioButtonUserMulti_Click(object sender, EventArgs e)
        {
            StageUserPreferences data = getUserPreferencesData();
            data.IsUserMulti = true;
        }
    }
}

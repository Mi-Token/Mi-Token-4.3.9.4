using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Microsoft.Win32;
using API_Autoconfig;

using System.ServiceProcess;

namespace API_setup
{

    //SetupConfigs for API_Setup
    
    public partial class Form2 : Form
    {
        private ConfigControl cfgControl;
        private int formHeightOffset = 90 + 6; //extra padding for buttons and just padding
        private int formWidthOffset = 20; //extra padding
        private APISetupConfigs settings;
        public Form2()
        {
            InitializeComponent();

            try
            {
                RegistryKey key = Registry.LocalMachine.CreateSubKey(@"Software\Mi-Token\Logon", RegistryKeyPermissionCheck.ReadWriteSubTree);
                key.Close();
            }

            catch (Exception ex)
            {
                Program.sufficientprivleges = false;
                MessageBox.Show("Insufficient privileges to access the registry, try running the program as an administrator", "Error: Can't Edit Registry");
                this.Close();
                return;
            }

            settings = new APISetupConfigs();
            cfgControl = new ConfigControl(settings);
            this.Width = ConfigControl.suggestedWidth + formWidthOffset;
            this.Height = ConfigControl.suggestedHeight + formHeightOffset;
            cfgControl.Parent = this;
            cfgControl.Location = new Point(3, 3);
            cfgControl.Size = new Size(this.Width - formWidthOffset, this.Height - formHeightOffset);
            cfgControl.Anchor = AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right | AnchorStyles.Top;
            this.Controls.Add(cfgControl);
            cfgControl.Show();
            cfgControl.Init(false);
        }

       
        private void Form2_Load(object sender, EventArgs e)
        {
            
        }

        

        private void button3_Click(object sender, EventArgs e)
        {

            if (settings.auth.serverTable.Rows.Count < 1)
            {
                Program.servers = false;
                if (Program.sufficientprivleges == true)
                {
                    MessageBox.Show("Please add at least one server", "Error: No Servers added");
                }
            }
            else
            {
                Program.servers = true;
                if (!cfgControl.bypassTable.hasBypass)
                {
                    if (MessageBox.Show("You have not configured any bypass codes. Proceed anyway?", "Warning", MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == DialogResult.No)
                    {
                        return;
                        /*
                        bypass b = new bypass();
                        b.ShowDialog();
                        bypassConfigured = b.hasBypass;
                        if (!bypassConfigured)
                            return;
                         * */
                    }
                }
                //set the enabled registy key to enable 2FA.
                RegistryKey RegKey = Registry.LocalMachine.OpenSubKey(@"Software\Mi-Token\Logon", true);
                RegKey.SetValue("Enabled", (int)1);
                RegKey.Close();
                Close();
            }
        }
    }

    public class APISetupConfigs : API_Autoconfig.ConfigControl.ICPSettings
    {
        new public class Auth : API_Autoconfig.ConfigControl.ICPSettings.Auth
        {
            override public int GetServerCount()
            {
                return serverCache.Count;
            }
            override public string GetServerName(int serverID)
            {
                Dictionary<int, string>.KeyCollection keys = serverCache.Keys;
                foreach (int key in keys)
                {
                    if (serverID == 0)
                    {
                        return serverCache[key];
                    }
                    serverID--;
                }

                return "";
            }
            public override int GetServerNameID(int serverID)
            {
                Dictionary<int, string>.KeyCollection keys = serverCache.Keys;
                foreach (int key in keys)
                {
                    if (serverID == 0)
                    {
                        return key;
                    }
                    serverID--;
                }

                return -1;
            }

            public override void addServer(int serverID, string serverName)
            {
                RegistryKey APIServerList = Registry.LocalMachine.CreateSubKey(@"Software\Mi-Token\Logon", RegistryKeyPermissionCheck.ReadWriteSubTree);
                APIServerList.SetValue("Server" + serverID, serverName);
            }
            public override void editServer(int serverID, string serverName)
            {
                RegistryKey APIServerList = Registry.LocalMachine.CreateSubKey(@"Software\Mi-Token\Logon", RegistryKeyPermissionCheck.ReadWriteSubTree);
                APIServerList.SetValue("Server" + serverID, serverName);
            }
            public override void removeServer(int serverID)
            {
                RegistryKey APIServerList = Registry.LocalMachine.CreateSubKey(@"Software\Mi-Token\Logon", RegistryKeyPermissionCheck.ReadWriteSubTree);
                APIServerList.DeleteValue("Server" + serverID);
            }
            public Auth()
            {
                bool skip = false;
                RegistryKey APIServerList = null;
                try
                {
                    APIServerList = Registry.LocalMachine.CreateSubKey(@"Software\Mi-Token\Logon", RegistryKeyPermissionCheck.ReadWriteSubTree);
                }
                catch
                {
                    Program.sufficientprivleges = false;
                    skip = true;
                }
                Dictionary<int, string> serverStrs = new Dictionary<int, string>();
                if (!skip)
                {
                    foreach (string ValueName in APIServerList.GetValueNames())
                    {
                        if (ValueName.StartsWith("Server"))
                        {
                            serverStrs.Add(Convert.ToInt32(ValueName.Substring(6)),(string)APIServerList.GetValue(ValueName));
                        }
                    }
                }
                serverCache = serverStrs;
            }

            protected Dictionary<int, string> serverCache;
        }
        new public class Bypass : API_Autoconfig.ConfigControl.ICPSettings.Bypass
        {
            public Bypass()
            {
                hashTable = new API_setup.BypassFromRegistry();
            }

        }
        new public class Filter : API_Autoconfig.ConfigControl.ICPSettings.Filter
        {
            private const string keyLocation = @"Software\Mi-Token\Logon";
            private const string valueLocal = "filterLocal";
            private const string valueRDP = "filterRDP";

            //private SettingConfig.filterModes cacheLocal, cacheRDP;

            public Filter()
            {
                readValues();
            }

            public override void readValues()
            {
                RegistryKey key = Registry.LocalMachine.OpenSubKey(keyLocation);
                cacheLocal = (ConfigControl.filterModes)key.GetValue(valueLocal, (int)ConfigControl.filterModes.Filter_Default);
                cacheRemote = (ConfigControl.filterModes)key.GetValue(valueRDP, (int)ConfigControl.filterModes.Filter_Default);
                key.Close();
            }

            public override void writeValues()
            {
                RegistryKey key = Registry.LocalMachine.OpenSubKey(keyLocation, true);
                key.SetValue(valueLocal, (int)cacheLocal);
                key.SetValue(valueRDP, (int)cacheRemote);
                key.Close();
            }
        }
        new public class Settings : API_Autoconfig.ConfigControl.ICPSettings.Settings
        {
            private const string keyLocationAdv = @"Software\Mi-Token\Logon\Config";
            private const string keyLocation = @"Software\Mi-Token\Logon";


            public Settings()
            {
                readValues();
            }

            public override void readValues()
            {
                //read all data

                //Advanced settings
                RegistryKey key = Registry.LocalMachine.OpenSubKey(keyLocationAdv);
                if (key != null)
                {
                    int value = (int)key.GetValue("APIUseHTTP", 0);
                    cacheHTTP = (value > 0);
                    value = (int)key.GetValue("DynamicPassword", 0);
                    cacheDynamic = (value > 0);
                    value = (int)key.GetValue("HideLastLoggedInUsername", 0);
                    cacheHideLastUsername = (value > 0);
                    key.Close();
                }
                else
                {
                    cacheHTTP = false;
                    cacheDynamic = false;
                }
                //Normal settings
                key = Registry.LocalMachine.OpenSubKey(keyLocation);
                if(key != null)
                {
                    int value = (int)key.GetValue("CPDebugMode", 0);
                    debugValue = value;
                    key.Close();
                }
                else
                {   
                    debugValue = 0;
                }

                //TODO : redo support for CP in Safemode.
                cacheForceCP = false;

                //TODO : Figure out where Cache Dump setting is stored
                cacheCacheDump = false;                
            }

            public override void writeValues()
            {
                //writes all data

                //Advanced settings
                RegistryKey key = Registry.LocalMachine.OpenSubKey(keyLocationAdv, true);
                if (key == null)
                {
                    key = Registry.LocalMachine.CreateSubKey(keyLocationAdv);
                }
                key.SetValue("APIUseHTTP", (cacheHTTP ? 1 : 0));
                key.SetValue("DynamicPassword", (cacheDynamic ? 1 : 0));
                key.SetValue("HideLastLoggedInUsername", (cacheHideLastUsername ? 1 : 0));
                key.Close();
                //Normal settings
                key = Registry.LocalMachine.OpenSubKey(keyLocation, true);
                if (key == null)
                {
                    key = Registry.LocalMachine.CreateSubKey(keyLocation);
                }
                key.SetValue("CPDebugMode", debugValue);

                key.Close();
            }

            
        }
        new public class TokenCache : API_Autoconfig.ConfigControl.ICPSettings.TokenCache
        {
            
            protected const string KeyLocation = @"Software\Mi-Token\Logon\Config";


            public override void readValues()
            {
                RegistryKey key = Registry.LocalMachine.OpenSubKey(KeyLocation);
                if (key != null)
                {
                    cacheTOTP = (int)key.GetValue("OTPCacheTOTP", 0);
                    cacheHOTP = (int)key.GetValue("OTPCacheHOTP", 0);
                    cachePoll = (int)key.GetValue("CachePollInSeconds", 0) / 60;
                    cacheRefresh = (int)key.GetValue("CacheClientWaitInMin", 0) / 60;
                    cacheName = (string)key.GetValue("CacheServerCustomerName", "");
                    cacheAddress = (string)key.GetValue("CacheServerAddress", "mobile.mi-token.com/v5cache");
                    cacheEnabled = (bool)(((int)key.GetValue("EnableOTPCaching", 0)) > 0);

                    key.Close();
                }
                else
                {
                    cacheTOTP = 0;
                    cacheHOTP = 0;
                    cachePoll = 0;
                    cacheRefresh = 0;
                    cacheName = "";
                    cacheAddress = "mobile.mi-token.com/v5cache";
                    cacheEnabled = false;
                }
            }
            public override void writeValues()
            {
                RegistryKey key = Registry.LocalMachine.OpenSubKey(KeyLocation, true);
                if (key == null)
                {
                    key = Registry.LocalMachine.CreateSubKey(KeyLocation);
                }
                key.SetValue("OTPCacheTOTP", cacheTOTP);
                key.SetValue("OTPCacheHOTP", cacheHOTP);
                //key.SetValue("CachePollInSeconds", cachePoll * 60);       // set by installer
                //key.SetValue("CacheClientWaitInMin", cacheRefresh * 60);  // set by installer
                //key.SetValue("CacheServerCustomerName", cacheName);       // set by installer
                key.SetValue("CacheServerAddress", cacheAddress);
                key.SetValue("EnableOTPCaching", cacheEnabled ? 1 : 0);
                key.Close();
            }

            public TokenCache()
            {
                readValues();
            }

            public override bool serviceInstalled()
            {
                ServiceController ctl = ServiceController.GetServices().FirstOrDefault(s => s.ServiceName == "Mi-Token Client Cache Updater");
                if (ctl == null)
                {
                    return false;
                }
                return true;
            }

            
        }

        public APISetupConfigs()
        {
            auth = new Auth();
            bypass = new Bypass();
            filter = new Filter();
            settings = new Settings();
            tokenCache = new TokenCache();
        }
    }

}

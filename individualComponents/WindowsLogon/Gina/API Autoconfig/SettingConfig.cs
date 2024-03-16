using System;
using System.Collections.Generic;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using API_setup;

namespace API_Autoconfig
{
    public partial class ConfigControl : UserControl
    {
        public const int suggestedWidth = 602;
        public const int suggestedHeight = 551;
        BypassTable bt = null;
        bool inInit = false;

        internal void refresh()
        {
            bt.reloadTable();
            radConLocal_CheckedChanged(null, null);
            radConRemote_CheckedChanged(null, null);
            radConBoth_CheckedChanged(null, null);
        }

        public BypassTable bypassTable
        {
            get
            {
                return bt;
            }
        }

        private ICPSettings settingsIO;

        public abstract class ICPSettings
        {
            public abstract class Auth
            {
                public DataTable serverTable = new DataTable();
                public BindingSource bs = new BindingSource();
                abstract public int GetServerCount();
                abstract public string GetServerName(int serverID);
                abstract public int GetServerNameID(int serverID);
                abstract public void addServer(int serverID, string serverName);
                abstract public void removeServer(int serverID);
                abstract public void editServer(int serverID, string serverName);
            };
            public abstract class Bypass
            {
                public IBypassConfig hashTable;
            }
            public abstract class Filter
            {
                public abstract void readValues();
                public abstract void writeValues();

                public filterModes cacheLocal, cacheRemote;
                public filterModes filterLocal
                {
                    get 
                    {
                        return cacheLocal;
                    }
                    set 
                    {
                        if (value != cacheLocal)
                        {
                            cacheLocal = value;
                            writeValues();
                        }
                    }
                }
                public filterModes filterRemote
                {
                    get
                    {
                        return cacheRemote;
                    }
                    set 
                    {
                        if (value != cacheRemote)
                        {
                            cacheRemote = value;
                            writeValues();
                        }
                    }
                }
            }
            public abstract class Settings
            {
                abstract public void readValues();
                abstract public void writeValues();

                public bool cacheForceCP, cacheCacheDump, cacheDebug, cacheDynamic, cacheSensitive, cacheVerbose, cacheHTTP, cacheHideLastUsername;

                protected int debugValue
                {
                    get
                    {
                        return (cacheDebug ? 1 : 0) + (cacheVerbose ? 2 : 0) + (cacheSensitive ? 4 : 0);
                    }
                    set
                    {
                        cacheDebug = ((value & 1) > 0);
                        cacheVerbose = ((value & 2) > 0);
                        cacheSensitive = ((value & 4) > 0);
                    }
                }
                public bool forceCPInSafeMode
                {
                    get
                    {
                        return cacheForceCP;
                    }
                    set
                    {
                        if (value != cacheForceCP)
                        {
                            cacheForceCP = value;
                            writeValues();
                        }
                    }
                }
                public bool enableCacheDumps
                {
                    get
                    {
                        return cacheCacheDump;
                    }
                    set
                    {
                        if (value != cacheCacheDump)
                        {
                            cacheCacheDump = value;
                            writeValues();
                        }
                    }
                }
                public bool enableDebug
                {
                    get
                    {
                        return cacheDebug;
                    }
                    set
                    {
                        if (value != cacheDebug)
                        {
                            cacheDebug = value;
                            writeValues();
                        }
                    }
                }
                public bool enableDynamic
                {
                    get
                    {
                        return cacheDynamic;
                    }
                    set
                    {
                        if (value != cacheDynamic)
                        {
                            cacheDynamic = value;
                            writeValues();
                        }
                    }
                }
                public bool enableSensitive
                {
                    get
                    {
                        return cacheSensitive;
                    }
                    set
                    {
                        if (value != cacheSensitive)
                        {
                            cacheSensitive = value;
                            writeValues();
                        }
                    }
                }
                public bool enableVerbose
                {
                    get
                    {
                        return cacheVerbose;
                    }
                    set
                    {
                        if (value != cacheVerbose)
                        {
                            cacheVerbose = value;
                            writeValues();
                        }
                    }
                }
                public bool useHTTPforAPI
                {
                    get
                    {
                        return cacheHTTP;
                    }
                    set
                    {
                        if (value != cacheHTTP)
                        {
                            cacheHTTP = value;
                            writeValues();
                        }
                    }
                }
                public bool hideLastUsername
                {
                    get
                    {
                        return cacheHideLastUsername;
                    }
                    set
                    {
                        if (value != cacheHideLastUsername)
                        {
                            cacheHideLastUsername = value;
                            writeValues();
                        }
                    }
                }
            }
            public abstract class TokenCache
            {
                public int cacheTOTP, cacheHOTP, cachePoll, cacheRefresh;
                public string cacheName, cacheAddress;
                public bool cacheEnabled;

                abstract public void readValues();
                abstract public void writeValues();
                abstract public bool serviceInstalled();

                public int clientRefresh
                {
                    get
                    {
                        return cacheRefresh / 60;
                    }
                    set
                    {
                        if ((value * 60) != cacheRefresh)
                        {
                            cacheRefresh = value * 60;
                            writeValues();
                        }
                    }
                }
                public string customerName
                {
                    get
                    {
                        return cacheName;
                    }
                    set
                    {
                        if (value != cacheName)
                        {
                            cacheName = value;
                            writeValues();
                        }
                    }
                }
                public bool enabled
                {
                    get
                    {
                        return cacheEnabled;
                    }
                    set
                    {
                        if (value != cacheEnabled)
                        {
                            cacheEnabled = value;
                            writeValues();
                        }
                    }
                }
                public int pollPeriod
                {
                    get
                    {
                        return cachePoll / 60;
                    }
                    set
                    {
                        if ((value * 60) != cachePoll)
                        {
                            cachePoll = value * 60;
                            writeValues();
                        }
                    }
                }
                public string serverAddress
                {
                    get
                    {
                        return cacheAddress;
                    }
                    set
                    {
                        if (value != cacheAddress)
                        {
                            cacheAddress = value;
                            writeValues();
                        }
                    }
                }
                public int tokenHOTPCount
                {
                    get
                    {
                        return cacheHOTP;
                    }
                    set
                    {
                        if (value != cacheHOTP)
                        {
                            cacheHOTP = value;
                            writeValues();
                        }
                    }
                }
                public int tokenTOTPCount
                {
                    get
                    {
                        return cacheTOTP;
                    }
                    set
                    {
                        if (value != cacheTOTP)
                        {
                            cacheTOTP = value;
                            writeValues();
                        }
                    }
                }
                
            }
            public Auth auth;
            public Bypass bypass;
            public Filter filter;
            public Settings settings;
            public TokenCache tokenCache;
        };

        public ConfigControl(ICPSettings mySettingsIO)
        {
            inInit = true;
            settingsIO = mySettingsIO;
            
            InitializeComponent();

            bt = new BypassTable();
            bt.ShowOKButton = false;
            bt.Size = new Size(groupBypass.Width - 6, groupBypass.Height - 16);
            bt.Location = new Point(3, 13);
            bt.Visible = true;
            bt.Anchor = AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right | AnchorStyles.Top;
            groupBypass.Controls.Add(bt);

            initAPIServers();
            initBypassTable();
            initFilterImages();
            initSettings();

            radConLocal.Checked = true;
        }

        public void Init(bool autoConfigMode)
        {
            inInit = true;
            if (autoConfigMode)
            {
                //disable the Token Caching Setting screen
                groupTokenCacheDisable.Visible = true;
                groupTokenCacheDisable.Enabled = true;
                groupTokenCacheEnabled.Visible = false;
            }
            else
            {
                //make sure the Token Caching Setting screen is enabled
                groupTokenCacheEnabled.Visible = true;
                groupTokenCacheDisable.Visible = false;
                groupTokenCacheEnabled.Enabled = true;
                initCache();
            }
            inInit = false;
        }

        private void ConfigControl_Load(object sender, EventArgs e)
        {

        }

        #region FilterMode Code
        public enum filterModes
        {
            Filter_None = 0,
            Filter_Default = 1,
            Filter_MiToken = 2,
            Filter_All = 3,
        };

        Bitmap filter0 = null, filter1 = null, filter2 = null, filter3 = null;
        bool allowIndeterminate;     

        private void initFilterImages()
        {
            try
            {
#if APISETUP
                //This is the APISETUP project, so the resources are embedded under API_setup.Resource1.*
                filter0 = new Bitmap(API_setup.Resource1.FilterMode0, pictureBox1.Size);
                filter1 = new Bitmap(API_setup.Resource1.FilterMode1, pictureBox1.Size);
                filter2 = new Bitmap(API_setup.Resource1.FilterMode2, pictureBox1.Size);
                filter3 = new Bitmap(API_setup.Resource1.FilterMode3, pictureBox1.Size);
#else
                //This is the AutoConfig project, so the resources are embedded under Resource1.*
                filter0 = new Bitmap(Resource1.FilterMode0, pictureBox1.Size);
                filter1 = new Bitmap(Resource1.FilterMode1, pictureBox1.Size);
                filter2 = new Bitmap(Resource1.FilterMode2, pictureBox1.Size);
                filter3 = new Bitmap(Resource1.FilterMode3, pictureBox1.Size);
#endif
            }
            catch
            {
                if (filter0 == null)
                    filter0 = new Bitmap(pictureBox1.Width, pictureBox1.Height);
                if (filter1 == null)
                    filter1 = new Bitmap(pictureBox1.Width, pictureBox1.Height);
                if (filter2 == null)
                    filter2 = new Bitmap(pictureBox1.Width, pictureBox1.Height);
                if (filter3 == null)
                    filter3 = new Bitmap(pictureBox1.Width, pictureBox1.Height);
            }

            allowIndeterminate = false;

            /*
            using (Microsoft.Win32.RegistryKey keyRead = Microsoft.Win32.Registry.LocalMachine.OpenSubKey("SOFTWARE\\Mi-Token\\Logon", Microsoft.Win32.RegistryKeyPermissionCheck.Default))
            {
                if (keyRead != null)
                {
                    if (Microsoft.Win32.Registry.GetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Mi-Token\\Logon", "filterLocal", "Not Exist").ToString() != "Not Exist")
                        settings.filter.filterCPLocal = (filterModes)keyRead.GetValue("filterLocal");
                    if (Microsoft.Win32.Registry.GetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Mi-Token\\Logon", "filterRDP", "Not Exist").ToString() != "Not Exist")
                        settings.filter.filterCPRemote = (filterModes)keyRead.GetValue("filterRDP");

                    settings.filter.filterXPLocal = (settings.filter.filterCPLocal == filterModes.Filter_MiToken ? filterModes.Filter_MiToken : filterModes.Filter_Default);
                    settings.filter.filterXPRemote = (settings.filter.filterCPRemote == filterModes.Filter_MiToken ? filterModes.Filter_MiToken : filterModes.Filter_Default);
                }
            }
             * */
        }

        private void saveFilterSetings(filterModes value)
        {
            if ((radConLocal.Checked) || (radConBoth.Checked))
            {
                settingsIO.filter.filterLocal = value;
            }
            if ((radConRemote.Checked) || (radConBoth.Checked))
            {
                settingsIO.filter.filterRemote = value;
            }
        }

        private void radFilter_CheckedChanged(object sender, EventArgs e)
        {
            if (radFilterNothing.Checked)
            {
                try
                {
                    pictureBox1.Image = filter0;
                }
                catch { }

                if (sender != null)
                  saveFilterSetings(filterModes.Filter_None);
            }
            if (radFilterDefault.Checked)
            {
                try
                {
                    pictureBox1.Image = filter1;
                }
                catch { }

                if (sender != null)
                  saveFilterSetings(filterModes.Filter_Default);
            }
            if (radFilterMiToken.Checked)
            {
                try
                {
                    pictureBox1.Image = filter2;
                }
                catch { }

                if (sender != null)
                  saveFilterSetings(filterModes.Filter_MiToken);
            }
            if (radFilterAll.Checked)
            {
                try
                {
                    pictureBox1.Image = filter3;
                }
                catch { }

                if (sender != null)
                  saveFilterSetings(filterModes.Filter_All);
            }
        }

        private void radConLocal_CheckedChanged(object sender, EventArgs e)
        {
            if (radConLocal.Checked == false)
                return;
            switch (settingsIO.filter.filterLocal)
            {
                case filterModes.Filter_None:
                    radFilterNothing.Checked = true;
                    break;
                case filterModes.Filter_Default:
                    radFilterDefault.Checked = true;
                    break;
                case filterModes.Filter_MiToken:
                    radFilterMiToken.Checked = true;
                    break;
                case filterModes.Filter_All:
                    radFilterAll.Checked = true;
                    break;
            }
            radFilter_CheckedChanged(sender, e);
        }

        private void radConRemote_CheckedChanged(object sender, EventArgs e)
        {
            if (radConRemote.Checked == false && sender != null)
                return;
            switch (settingsIO.filter.filterRemote)
            {
                case filterModes.Filter_None:
                    radFilterNothing.Checked = true;
                    break;
                case filterModes.Filter_Default:
                    radFilterDefault.Checked = true;
                    break;
                case filterModes.Filter_MiToken:
                    radFilterMiToken.Checked = true;
                    break;
                case filterModes.Filter_All:
                    radFilterAll.Checked = true;
                    break;
            }
            radFilter_CheckedChanged(sender, e);
        }

        private void radConBoth_CheckedChanged(object sender, EventArgs e)
        {
            if (radConBoth.Checked == false)
                return;
            //only show an optino if the two values are the same
            if (settingsIO.filter.filterRemote == settingsIO.filter.filterLocal)
            {
                radConRemote_CheckedChanged(sender, e);
            }
            else
            {
                radFilterAll.Checked = false;
                radFilterDefault.Checked = false;
                radFilterMiToken.Checked = false;
                radFilterNothing.Checked = false;
                pictureBox1.Image = null;
            }

            //TODO : what is this meant to do?
            allowIndeterminate = true;
            allowIndeterminate = false;
            radFilter_CheckedChanged(sender, e);
        }

        private void checkXPOTP_CheckStateChanged(object sender, EventArgs e)
        {
            if (checkXPOTP.CheckState == CheckState.Indeterminate)
            {
                if (!allowIndeterminate)
                {
                    checkXPOTP.CheckState = CheckState.Unchecked;
                    checkXPOTP_CheckStateChanged(sender, e);
                }
                return;
            }
        }


        #endregion

        #region API Servers

        private void initAPIServers()
        {
            settingsIO.auth.serverTable = new DataTable();
            settingsIO.auth.serverTable.Columns.Add("Trial Order");
            settingsIO.auth.serverTable.Columns.Add("IP Address / Hostname");

            settingsIO.auth.bs.DataSource = settingsIO.auth.serverTable;

            dataGridViewRADIUSServers.DataSource = settingsIO.auth.bs;

            int serverCount = settingsIO.auth.GetServerCount();
            for (int i = 0; i < serverCount; ++i)
            {
                settingsIO.auth.serverTable.Rows.Add(settingsIO.auth.GetServerNameID(i), settingsIO.auth.GetServerName(i));
            }

        }

        private void btnAddRadius_Click(object sender, EventArgs e)
        {
            AddServer AS = new AddServer();
            var dlg = AS.ShowDialog();

            if (dlg != DialogResult.OK || string.IsNullOrWhiteSpace(AS.IPAddress))
                return;

            int serverN = 0;
            if(dataGridViewRADIUSServers.Rows.Count != 0)
            {
                DataGridViewRow Lrow = dataGridViewRADIUSServers.Rows[settingsIO.auth.serverTable.Rows.Count - 1];
                serverN = Convert.ToInt32(Lrow.Cells[0].Value);
                serverN++;
            }
            else
            {
                serverN = 0;
            }
            settingsIO.auth.addServer(serverN, AS.IPAddress);
            settingsIO.auth.serverTable.Rows.Add(serverN, AS.IPAddress);
            
        }

        private void btnEditRadius_Click(object sender, EventArgs e)
        {
            DataGridViewSelectedRowCollection row = dataGridViewRADIUSServers.SelectedRows;
            if (row.Count == 0)
                return; //no rows selected, do nothing
            AddServer dlg = new AddServer();
            dlg.setIP(row[0].Cells[1].Value.ToString());

            if (dlg.ShowDialog(this) == DialogResult.OK)
            {
                row[0].Cells[1].Value = dlg.IPAddress;

                settingsIO.auth.editServer(Convert.ToInt32(row[0].Cells[0].Value), dlg.IPAddress);
            }           
        }

        private void btnDeleteRadius_Click(object sender, EventArgs e)
        {
            DataGridViewSelectedRowCollection row = dataGridViewRADIUSServers.SelectedRows;
            settingsIO.auth.removeServer(Convert.ToInt32(row[0].Cells[0].Value));
            settingsIO.auth.serverTable.Rows.RemoveAt(row[0].Index);

        }

        private void button3_Click(object sender, EventArgs e)
        {
            DataGridViewSelectedRowCollection row = dataGridViewRADIUSServers.SelectedRows;
            tester dlg = new tester();
            if (row.Count > 0)
            {
                dlg.txtboxAPIServer.Text = row[0].Cells[1].Value.ToString();
                dlg.ShowDialog();
            }
        }

        #endregion

        #region Bypass Table

        private void initBypassTable()
        {
            bt.hashTable = settingsIO.bypass.hashTable;
        }

        #endregion 

        #region Settings

        private void initSettings()
        {
            checkCredInSafeMode.Checked = settingsIO.settings.forceCPInSafeMode;
            checkUseHTTP.Checked = settingsIO.settings.useHTTPforAPI;
            checkTraceLogging.Checked = settingsIO.settings.enableDebug;
            checkVerboseLogging.Checked = settingsIO.settings.enableVerbose;
            checkSensitiveLogging.Checked = settingsIO.settings.enableSensitive;
            checkCacheDumps.Checked = settingsIO.settings.enableCacheDumps;
            checkDynamicPassword.Checked = settingsIO.settings.enableDynamic;
            checkHideLastUsername.Checked = settingsIO.settings.hideLastUsername;
           
        }

        #endregion

        #region Cache Settings
        private void initCache()
        {
            if (settingsIO.tokenCache.serviceInstalled())
            {
                labStatusText.Text = "Service Installed";
            }
            else
            {
                labStatusText.Text = "Caution: Cache Syncing Service Is Not Installed";
            }

            txtTOTPCount.Text = settingsIO.tokenCache.tokenTOTPCount.ToString();
            txtHOTPCount.Text = settingsIO.tokenCache.tokenHOTPCount.ToString();
            txtCustomerName.Text = settingsIO.tokenCache.customerName;
            txtServerAddress.Text = settingsIO.tokenCache.serverAddress;
            txtPollPeriod.Text = settingsIO.tokenCache.pollPeriod.ToString();
            txtClientRefresh.Text = settingsIO.tokenCache.clientRefresh.ToString();
            checkEnableCaching.Checked = settingsIO.tokenCache.cacheEnabled;
        }

        private void txtTOTPCount_TextChanged(object sender, EventArgs e)
        {
            int v;
            if (Int32.TryParse(txtTOTPCount.Text, out v))
            {
                settingsIO.tokenCache.tokenTOTPCount = v;
            }
        }

        private void txtHOTPCount_TextChanged(object sender, EventArgs e)
        {
            int v;
            if (Int32.TryParse(txtHOTPCount.Text, out v))
            {
                settingsIO.tokenCache.tokenHOTPCount = v;
            }
        }


        private void txtClientRefresh_TextChanged(object sender, EventArgs e)
        {
            int v;
            if (Int32.TryParse(txtClientRefresh.Text, out v))
            {
                settingsIO.tokenCache.clientRefresh = v;
            }

        }

        private void txtPollPeriod_TextChanged(object sender, EventArgs e)
        {
            int v;
            if (Int32.TryParse(txtPollPeriod.Text, out v))
            {
                settingsIO.tokenCache.pollPeriod = v;
            }
        }

        private void txtServerAddress_TextChanged(object sender, EventArgs e)
        {
            settingsIO.tokenCache.serverAddress = txtServerAddress.Text;
        }

        private void txtCustomerName_TextChanged(object sender, EventArgs e)
        {
            settingsIO.tokenCache.customerName = txtCustomerName.Text;
        }

        #endregion

        private void checkTraceLogging_CheckedChanged(object sender, EventArgs e)
        {
            if (checkTraceLogging.Checked == false)
            {
                settingsIO.settings.enableSensitive = false;
                checkSensitiveLogging.Checked = false;
            }
            settingsIO.settings.enableDebug = checkTraceLogging.Checked;
            
        }

        private void checkEnableCaching_CheckedChanged(object sender, EventArgs e)
        {
            settingsIO.tokenCache.enabled = checkEnableCaching.Checked;
        }

        private void checkCredInSafeMode_CheckedChanged(object sender, EventArgs e)
        {
            settingsIO.settings.forceCPInSafeMode = checkCredInSafeMode.Checked;
        }

        private void checkUseHTTP_CheckedChanged(object sender, EventArgs e)
        {
            settingsIO.settings.useHTTPforAPI = checkUseHTTP.Checked;
        }

        private void checkVerboseLogging_CheckedChanged(object sender, EventArgs e)
        {
            settingsIO.settings.enableVerbose = checkVerboseLogging.Checked;
        }

        private void checkSensitiveLogging_CheckedChanged(object sender, EventArgs e)
        {
            if (!inInit)
            {
                if (checkSensitiveLogging.Checked)
                {
                    MessageBox.Show("Caution : Enabling sensitive logging will cause Mi-Token to log passwords and other sensitive information in plain text. This option should only be turned on if requested by Mi-Token support");
                }
            }
            settingsIO.settings.enableSensitive = checkSensitiveLogging.Checked;
            if (checkSensitiveLogging.Checked)
            {
                checkTraceLogging.Checked = true;
            }
        }

        private void checkCacheDumps_CheckedChanged(object sender, EventArgs e)
        {
            settingsIO.settings.enableCacheDumps = checkCacheDumps.Checked;
        }

        private void checkDynamicPassword_CheckedChanged(object sender, EventArgs e)
        {
            settingsIO.settings.enableDynamic = checkDynamicPassword.Checked;
        }

        private void tabPage3_Click(object sender, EventArgs e)
        {

        }

        private void groupTokenCacheEnabled_Enter(object sender, EventArgs e)
        {

        }

        private void groupBoxRadiusServer_Enter(object sender, EventArgs e)
        {

        }

        private void groupBypass_Enter(object sender, EventArgs e)
        {

        }

        private void tabPage4_Click(object sender, EventArgs e)
        {

        }

        private void checkHideLastUsername_Checked(object sender, EventArgs e)
        {
            settingsIO.settings.hideLastUsername = checkHideLastUsername.Checked;
        }

        
    }
}

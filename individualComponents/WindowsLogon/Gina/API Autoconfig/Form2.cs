using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Microsoft.Win32;
using System.IO;

using API_setup;
using System.Diagnostics;

namespace API_Autoconfig
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
            cfgControl.Init(true);
        }


        private void Form2_Load(object sender, EventArgs e)
        {
            
        }

        private void button2_Click(object sender, EventArgs e)
        {
            OpenFileDialog OFD = new OpenFileDialog();
            OFD.Filter = "Mi-Token Bypass Autoconfig Files (*.acf) | *.acf";
            PersistSettings.Settings readSettings = null;
            if (OFD.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                try
                {
                    StreamReader sr = new StreamReader(OFD.FileName);

                    string nstr = PersistSettings.readXMLFromStream(sr, out readSettings);
                    if (nstr != null)
                    {
                        MessageBox.Show(nstr, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        return;
                    }
                    sr.Close();
                }
                catch (Exception ex)
                {
                    MessageBox.Show("File load failed.\nMake sure it is a valid Autoconfig File\nError : " + ex.Message);
                    return;
                }
                if (readSettings != null)
                {
                    settings.auth.serverTable.Rows.Clear();
                    foreach (string server in readSettings.servers)
                    {
                        settings.auth.serverTable.Rows.Add(settings.auth.serverTable.Rows.Count, server);
                    }
                    settings.bypass.hashTable.removeAllHashes();
                    foreach (var bypass in readSettings.bypass)
                    {
                        foreach (var hashStr in bypass.bypassCodes)
                        {
                            byte[] hash = Convert.FromBase64String(hashStr);
                            settings.bypass.hashTable.appendHash(bypass.userSID, hash);
                        }
                    }
                    settings.filter.cacheLocal = (ConfigControl.filterModes)readSettings.filterCPLocal;
                    settings.filter.cacheRemote = (ConfigControl.filterModes)readSettings.filterCPRemote;
                }
                cfgControl.refresh();
            }
        }

        private void butAddToAutosetup_Click(object sender, EventArgs e)
        {
            string appPath = Path.GetDirectoryName(Application.ExecutablePath);
            if (!appPath.EndsWith("\\"))
                appPath += "\\";
            bool usingAutoFileLocation = false;
            if (!File.Exists(appPath + "config.acf"))
            {
                usingAutoFileLocation = true;
                ACStr = appPath + "config.acf";
                saveACF(ACStr);
            }
            if (!usingAutoFileLocation)
            {
                butCreateACF_Click(sender, e);
                if (ACStr == "")
                    return;
            }
            string installerPath = appPath + "PackedInstaller.exe";
            if (!File.Exists(installerPath))
            {
                OpenFileDialog OFD = new OpenFileDialog();
                OFD.Filter = "Installation Application (*.exe) | *.exe";
                OFD.Title = "Please select the installation application to use";
                if (OFD.ShowDialog() == DialogResult.OK)
                    installerPath = OFD.FileName;
                else
                    return;
            }
            {
                /*              
                                Process proc = new Process();
                                proc.StartInfo.FileName = installerPath;
                                proc.StartInfo.Arguments = "-GetInfo";
                                proc.Start();
                                proc.WaitForExit();
                                if ((proc.ExitCode & 0x04) == 0x04)
                                {
                                    if (MessageBox.Show("Warning : The Installer you selected already has an Auto Config Setting attached to it.\nOverwrite?", "Warning", MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == DialogResult.No)
                                    {
                                        return;
                                    }
                                }
                 */
                SaveFileDialog SFD = new SaveFileDialog();
                SFD.Filter = "Installation Application (*.exe) | *.exe";
                SFD.Title = "Please select an output location";
                do
                {
                    if (SFD.ShowDialog() == DialogResult.OK)
                    {
                        if (SFD.FileName == installerPath)
                        {
                            MessageBox.Show("Output location cannot be the same as the input location.");
                            continue;
                        }
                        break;
                    }
                    else
                    {
                        if (usingAutoFileLocation)
                            File.Delete(ACStr);
                        return;
                    }
                } while (true);
                Process proc = new Process();
                proc.StartInfo.FileName = installerPath;
                proc.StartInfo.Arguments = string.Format("-ac \"{0}\" -o \"{1}\"", ACStr, SFD.FileName);
                try
                {
                    proc.Start();
                    proc.WaitForExit();
                    switch (proc.ExitCode)
                    {
                        case 0:
                            MessageBox.Show("File successfully imported.");
                            break;
                        case -1:
                            MessageBox.Show("Invalid argument given.");
                            break;
                        case -2:
                            MessageBox.Show("Invalid number of arguments given.");
                            break;
                        case -3:
                            MessageBox.Show("Missing required argument [-o Path].");
                            break;
                        case -4:
                            MessageBox.Show("Failed to open the requested output file.");
                            break;
                        default:
                            MessageBox.Show("Unexpected return code " + proc.ExitCode);
                            break;
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show("An Error Occurred while trying to package the installer:\n" + ex.Message);
                }

                if (usingAutoFileLocation)
                    File.Delete(ACStr);
            }
        }

        private void butCreateACF_Click(object sender, EventArgs e)
        {
            SaveFileDialog SFD = new SaveFileDialog();
            SFD.Filter = "Mi-Token Bypass Autoconfig Files (*.acf) | *.acf";
            if (SFD.ShowDialog() == DialogResult.OK)
            {
                saveACF(SFD.FileName);
            }
            else
            {
                ACStr = "";
            }
        }
        string ACStr;

        private void saveACF(string filepath)
        {
            PersistSettings.Settings readSettings = new PersistSettings.Settings();
            readSettings.debugLogging = settings.settings.enableDebug;
            readSettings.filterXPLocal = 0;
            readSettings.filterXPRDP = 0;
            readSettings.filterCPLocal = (int)settings.filter.filterLocal;
            readSettings.filterCPRemote = (int)settings.filter.filterRemote;
            readSettings.useDynamicPasswords = (int)(settings.settings.enableDynamic ? 1 : 0);
            readSettings.useHTTP = (int)(settings.settings.useHTTPforAPI ? 1 : 0);
            readSettings.useCredInSafeMode = (int)(settings.settings.forceCPInSafeMode ? 1 : 0);
            readSettings.servers = new string[settings.auth.serverTable.Rows.Count];
            for (int i = 0; i < settings.auth.serverTable.Rows.Count; ++i)
            {
                readSettings.servers[i] = settings.auth.serverTable.Rows[i].ItemArray[1].ToString();
            }
            string[] usernames = settings.bypass.hashTable.getUsernames();
            readSettings.bypass = new PersistSettings.Settings.Bypass[usernames.Length];
            for (int i = 0; i < usernames.Length; ++i)
            {
                readSettings.bypass[i] = new PersistSettings.Settings.Bypass();
                readSettings.bypass[i].userSID = usernames[i];
                byte[] hashes = settings.bypass.hashTable.getHashes(usernames[i]);
                readSettings.bypass[i].bypassCodes = new string[(hashes.Length - 21) / 20];
                for (int j = 21; j < hashes.Length; j += 20)
                {
                    byte[] hash = new byte[20];
                    Array.Copy(hashes, j, hash, 0, 20);
                    readSettings.bypass[i].bypassCodes[(j - 21) / 20] = Convert.ToBase64String(hash);
                }
            }

            StreamWriter sw = new StreamWriter(filepath);
            PersistSettings.writeXMLToStream(sw, readSettings);
            sw.Close();
            //MessageBox.Show("File saved");
            ACStr = filepath;
            return;
        }
    }



    public class APISetupConfigs : API_Autoconfig.ConfigControl.ICPSettings
    {
        new public class Auth : API_Autoconfig.ConfigControl.ICPSettings.Auth
        {
            override public int GetServerCount()
            {
                return serverCache.Length;
            }
            override public string GetServerName(int serverID)
            {
                return serverCache[serverID];
            }
            public override int GetServerNameID(int serverID)
            {
                return serverID;
            }
            public override void addServer(int serverID, string serverName)
            {
                return;
            }
            public override void editServer(int serverID, string serverName)
            {
                return;
            }
            public override void removeServer(int serverID)
            {
                return;
            }

            public Auth()
            {
                serverCache = new string[0];
            }

            protected string[] serverCache;
        }
        new public class Bypass : API_Autoconfig.ConfigControl.ICPSettings.Bypass
        {
            public Bypass()
            {
                hashTable = new API_setup.BypassFromMemory();
            }

        }
        new public class Filter : API_Autoconfig.ConfigControl.ICPSettings.Filter
        {
            public override void readValues()
            {
                cacheLocal = ConfigControl.filterModes.Filter_Default;
                cacheRemote = ConfigControl.filterModes.Filter_Default;
            }
            public override void writeValues()
            {
                
            }
            public Filter()
            {
                readValues();
            }
        }
        new public class Settings : API_Autoconfig.ConfigControl.ICPSettings.Settings
        {
            public override void readValues()
            {
                cacheCacheDump = false;
                cacheDebug = false;
                cacheDynamic = false;
                cacheForceCP = false;
                cacheHTTP = false;
                cacheSensitive = false;
                cacheVerbose = false;
            }
            public override void writeValues()
            {
                
            }
            public Settings()
            {
            }
        }
        new public class TokenCache : API_Autoconfig.ConfigControl.ICPSettings.TokenCache
        {
            public override void readValues()
            {
                
            }
            public override void writeValues()
            {
                
            }
            public override bool serviceInstalled()
            {
                return false;
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

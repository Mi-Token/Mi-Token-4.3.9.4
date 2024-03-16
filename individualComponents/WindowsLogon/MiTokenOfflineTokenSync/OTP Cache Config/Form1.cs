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

using System.Diagnostics;

using System.ServiceProcess;

namespace OTP_Cache_Config
{
    public partial class Form1 : Form
    {

        bool readRegInt(string stringValue, out int value)
        {
            value = 0;
            if(stringValue.StartsWith("dword:"))
            {
                string s = stringValue.Substring(6);
                if(Int32.TryParse(s, System.Globalization.NumberStyles.HexNumber, new System.Globalization.CultureInfo("en-US"), out value))
                {
                    return true;
                }
                return false;
            }
            else
            {
                return false;
            }
        }

        string readRegString(string stringValue)
        {
            return stringValue.Substring(1, stringValue.Length - 2);
        }
        void readRegData(string regFile)
        {
            if (File.Exists(regFile))
            {
                FileStream stream = new FileStream(regFile, FileMode.Open);
                byte[] fileData = new byte[(int)stream.Length];
                stream.Read(fileData, 0, fileData.Length);
                stream.Close();
                string str = Encoding.ASCII.GetString(fileData);
                string[] strs = str.Split(new string[] { "\r\n" }, StringSplitOptions.RemoveEmptyEntries);
                bool clientMode = false;
                checkClientMachine.Checked = false;
                checkEnableCaching.Checked = true;
                int value;
                foreach (string line in strs)
                {
                    if (line.Contains('='))
                    {
                        string[] parts = line.Split(new string[] { "=" }, StringSplitOptions.None);

                        //only allow when we have (something)=(something)
                        if (parts.Length == 2)
                        {
                            switch (parts[0])
                            {
                                case "\"EnableOTPCaching\"":
                                    if (parts[1] == "dword:00000001")
                                    {
                                        checkClientMachine.Checked = true;
                                    }
                                    break;
                                case "\"OTPCacheTOTP\"":
                                    if (readRegInt(parts[1], out value))
                                    {
                                        txtTOTPCount.Text = value.ToString();
                                        clientMode = true;
                                    }
                                    break;
                                case "\"OTPCacheHOTP\"":
                                    if (readRegInt(parts[1], out value))
                                    {
                                        txtHOTPCount.Text = value.ToString();
                                        clientMode = true;
                                    }
                                    break;
                                case "\"CacheServerCustomerName\"":
                                    txtCustomerName.Text = readRegString(parts[1]);
                                    break;
                                case "\"CacheServerAddress\"":
                                    txtServerAddress.Text = readRegString(parts[1]);
                                    break;
                                case "\"CacheClientWaitInMin\"":
                                    if (readRegInt(parts[1], out value))
                                    {
                                        txtClientRefresh.Text = (value / 60).ToString();
                                        clientMode = true;
                                    }
                                    break;
                                case "\"CachePollInSeconds\"":
                                    if (readRegInt(parts[1], out value))
                                    {
                                        txtPollPeriod.Text = (value / 60).ToString();
                                    }
                                    break;
                            }
                        }
                    }
                }
                if (clientMode)
                    checkClientMachine.Checked = true;
            }
        }
        

        void readRegFileData()
        {
            string path = Application.StartupPath;
            string clientPath = path + "\\cacheClientInfo.reg";
            string serverPath = path + "\\cacheServerInfo.reg";
            if (!File.Exists(clientPath))
            {
                if (!File.Exists(serverPath))
                {
                    return;
                }
                //load client info
                readRegData(clientPath);
            }
            else
            {
                if (File.Exists(serverPath))
                {
                    //there is both a server and client version, ask the user which one they want.
                    if (MessageBox.Show("Found both client and server reg files.\r\nWould you like to load the Client version?\r\nYes = Load Client Version\r\nNo = Load Server Version", "Version Confirmation", MessageBoxButtons.YesNo) == System.Windows.Forms.DialogResult.Yes)
                    {
                        //load client info
                        readRegData(clientPath);
                    }
                    else
                    {
                        //load server info
                        readRegData(serverPath);
                    }
                }
                else
                {
                    //load server info
                    readRegData(serverPath);
                }
            }
        }

        public bool serverServiceInstalled()
        {
            //first check the override modes
            if (Program.serverMode)
            {
                return true;
            }
            if (Program.clientMode)
            {
                return false;
            }

            ServiceController ctl = ServiceController.GetServices().FirstOrDefault(s => s.ServiceName == "Mi-Token OTP Cache Server");
            if (ctl != null)
            {
                return true;
            }
            return false;
        }

        public bool clientServiceInstalled()
        {
            //first check the override modes
            if (Program.serverMode)
            {
                return false;
            }
            if (Program.clientMode)
            {
                return true;
            }


            ServiceController ctl = ServiceController.GetServices().FirstOrDefault(s => s.ServiceName == "Mi-Token Client Cache Updater");
            if (ctl != null)
            {
                return true;
            }
            return false;
        }

        public Form1()
        {
            InitializeComponent();
            readRegFileData();
            checkEnableCaching_CheckedChanged(null, new EventArgs());
            ToolTip toolTip = new ToolTip();
            toolTip.SetToolTip(this.checkEnableCaching, "Check to enable Mi-Token OTP Caching on this machine (Server and Client)");
            toolTip.SetToolTip(this.labCustomerName, "The unique name to distinguish your clients/servers from other customers.\n(Must be the same on every machine)");
            toolTip.SetToolTip(this.txtCustomerName, "The unique name to distinguish your clients/servers from other customers.\n(Must be the same on every machine)");

            toolTip.SetToolTip(this.labServerAddress, "The Mi-Token Provisioning Server that requests are made to.\n(Must be the same on every machine)");
            toolTip.SetToolTip(this.txtServerAddress, "The Mi-Token Provisioning Server that requests are made to.\n(Must be the same on every machine)");
            
            toolTip.SetToolTip(this.labPollPeriod, "How often this machine should poll the Provisioning Server. (Updates to the cache take 3 poll periods. Recommended value is 30)");
            toolTip.SetToolTip(this.txtPollPeriod, "How often this machine should poll the Provisioning Server. (Updates to the cache take 3 poll periods. Recommended value is 30)");
            
            toolTip.SetToolTip(this.checkClientMachine, "Check to enable editing and storing of client specific registry keys");
            
            toolTip.SetToolTip(this.labClientRefresh, "How often the client should start a new request to update its cache (Recommended value is 4)");
            toolTip.SetToolTip(this.txtClientRefresh, "How often the client should start a new request to update its cache (Recommended value is 4)");

            toolTip.SetToolTip(this.labTOTPCount, "How many future tokens to cache for time based tokens (Overrides server settings. Use 0 to use the server settings. Recommended value is 0)");
            toolTip.SetToolTip(this.txtTOTPCount, "How many future tokens to cache for time based tokens (Overrides server settings. Use 0 to use the server settings. Recommended value is 0)");

            toolTip.SetToolTip(this.labHOTPCount, "How many future tokens to cache for event based tokens (Overrides server settings. Use 0 to use the server settings. Recommended value is 0)");
            toolTip.SetToolTip(this.txtHOTPCount, "How many future tokens to cache for event based tokens (Overrides server settings. Use 0 to use the server settings. Recommended value is 0)");

            toolTip.SetToolTip(this.butSave, "Save the settings to this machines registry");
            toolTip.SetToolTip(this.butExport, "Generate a .reg file of the current settings");
            toolTip.SetToolTip(this.butInstallService, "Installs the Service and sets the registry keys on this machine");

            bool clientMachine = false;

            //Special case for if we are on a client enabled machine.
            if (clientServiceInstalled())
            {
                checkClientMachine.Checked = true;
                checkClientMachine.Enabled = false;
                clientMachine = true;
                labClientMachine.Text = "Client Service detected.";
            }
            
            try
            {
                RegistryKey key = OpenOrCreateKey(Registry.LocalMachine, "Software\\Mi-Token\\Logon\\Config\\");
                txtCustomerName.Text = (string)key.GetValue("CacheServerCustomerName", "");
                txtServerAddress.Text = (string)key.GetValue("CacheServerAddress", "mobile.mi-token.com/v5cache");
                int pollPeriod = (int)key.GetValue("CachePollInSeconds", 0);
                int clientRefresh = 0;
                int totpCount = 0, hotpCount = 0;
                if (clientMachine)
                {
                    clientRefresh = (int)key.GetValue("CacheClientWaitInMin", 0);
                    totpCount = (int)key.GetValue("OTPCacheTOTP", 0);
                    hotpCount = (int)key.GetValue("OTPCacheHOTP", 0);
                }

                txtHOTPCount.Text = hotpCount.ToString();
                txtTOTPCount.Text = totpCount.ToString();
                clientRefresh /= 60;
                pollPeriod /= 60;
                txtClientRefresh.Text = clientRefresh.ToString();
                txtPollPeriod.Text = pollPeriod.ToString();
            }
            catch
            {}
        }

        private void checkEnableCaching_CheckedChanged(object sender, EventArgs e)
        {
            bool enabled = checkEnableCaching.Checked;
            labCustomerName.Enabled = enabled;
            labServerAddress.Enabled = enabled;
            labPollPeriod.Enabled = enabled;

            txtCustomerName.Enabled = enabled;
            txtServerAddress.Enabled = enabled;
            txtPollPeriod.Enabled = enabled;

            checkClientMachine.Enabled = enabled;
            groupClientSettings.Enabled = enabled;
            checkClientMachine_CheckedChanged(sender, e);
        }

        private void checkClientMachine_CheckedChanged(object sender, EventArgs e)
        {
            bool enabled = checkClientMachine.Checked & checkEnableCaching.Checked;
            labClientRefresh.Enabled = enabled;
            labTOTPCount.Enabled = enabled;
            labHOTPCount.Enabled = enabled;
            txtClientRefresh.Enabled = enabled;
            txtTOTPCount.Enabled = enabled;
            txtHOTPCount.Enabled = enabled;
        }

        private void groupBox1_Enter(object sender, EventArgs e)
        {

        }

        private void labHOTPCount_Click(object sender, EventArgs e)
        {

        }

        private RegistryKey OpenOrCreateKey(RegistryKey baseAddress, string subKey)
        {
            RegistryKey result = baseAddress.OpenSubKey(subKey, true);
            if (result == null)
            {
                result = baseAddress.CreateSubKey(subKey);
            }
            return result;
        }

        private void butSave_Click(object sender, EventArgs e)
        {
            if (checkEnableCaching.Checked)
            {
                int pollPeriod = 0, clientRefresh = 0, totpCount = 0, hotpCount = 0;
                if(!Int32.TryParse(txtPollPeriod.Text, out pollPeriod))
                {
                    MessageBox.Show("Failed to set configuration : Invalid value set for Poll Period");
                    return;
                }
                if(checkClientMachine.Checked)
                {
                    if(!Int32.TryParse(txtClientRefresh.Text, out clientRefresh))
                    {
                        MessageBox.Show("Failed to set configuration : Invalid value set for Client Refresh");
                        return;
                    }
                    if(!Int32.TryParse(txtTOTPCount.Text, out totpCount))
                    {
                        MessageBox.Show("Failed to set configuration : Invalid value set for Token Cache Count (Time)");
                        return;
                    }
                    if(!Int32.TryParse(txtHOTPCount.Text, out hotpCount))
                    {
                        MessageBox.Show("Failed to set configuration : Invalid value set for Token Cache Count (Event)");
                        return;
                    }
                }


                clientRefresh *= 60;
                pollPeriod *= 60;

                RegistryKey key = OpenOrCreateKey(Registry.LocalMachine, "Software\\Mi-Token\\Logon\\Config\\");
                key.SetValue("CacheServerCustomerName", txtCustomerName.Text, RegistryValueKind.String);
                key.SetValue("CacheServerAddress", txtServerAddress.Text, RegistryValueKind.String);
                key.SetValue("CachePollInSeconds", pollPeriod, RegistryValueKind.DWord);
                if (checkClientMachine.Checked)
                {
                    key.SetValue("EnableOTPCaching", 1, RegistryValueKind.DWord);
                    key.SetValue("CacheClientWaitInMin", clientRefresh, RegistryValueKind.DWord);
                    key.SetValue("OTPCacheTOTP", totpCount, RegistryValueKind.DWord);
                    key.SetValue("OTPCacheHOTP", hotpCount, RegistryValueKind.DWord);
                }


                ServiceController ctl = ServiceController.GetServices().FirstOrDefault(s => s.ServiceName == "Mi-Token OTP Cache Server");
                if (ctl != null)
                {
                    //stop and start the service
                    if (ctl.Status == ServiceControllerStatus.Running)
                    {
                        ctl.Stop();
                    }
                    if (ctl.Status == ServiceControllerStatus.Stopped)
                    {
                        ctl.Start();
                    }
                }
                   
                /*
                else
                {
                    ServiceController ctl = ServiceController.GetServices().FirstOrDefault(s => s.ServiceName == "Mi-Token Server Cache Updater");
                    if (ctl != null)
                    {
                        ctl.Start(); //start the new server service
                    }
                    else
                    {
                        ctl = ServiceController.GetServices().FirstOrDefault(s => s.ServiceName == "Mi-Token OTP Cache Server");
                        ctl.Start(); //start the old server service
                    }
                }
                */
                MessageBox.Show("Settings updated");

                this.Close();
            }
            
        }

        private void butExport_Click(object sender, EventArgs e)
        {
            if (checkEnableCaching.Checked)
            {
                int pollPeriod = 0, clientRefresh = 0, totpCount = 0, hotpCount = 0;
                if (!Int32.TryParse(txtPollPeriod.Text, out pollPeriod))
                {
                    MessageBox.Show("Failed to set configuration : Invalid value set for Poll Period");
                    return;
                }
                if (checkClientMachine.Checked)
                {
                    if (!Int32.TryParse(txtClientRefresh.Text, out clientRefresh))
                    {
                        MessageBox.Show("Failed to set configuration : Invalid value set for Client Refresh");
                        return;
                    }
                    if (!Int32.TryParse(txtTOTPCount.Text, out totpCount))
                    {
                        MessageBox.Show("Failed to set configuration : Invalid value set for Token Cache Count (Time)");
                        return;
                    }
                    if (!Int32.TryParse(txtHOTPCount.Text, out hotpCount))
                    {
                        MessageBox.Show("Failed to set configuration : Invalid value set for Token Cache Count (Event)");
                        return;
                    }
                }

                clientRefresh *= 60;
                pollPeriod *= 60;

                StringBuilder fileData = new StringBuilder();
                fileData.Append("Windows Registry Editor Version 5.00\r\n\r\n[HKEY_LOCAL_MACHINE\\SOFTWARE\\Mi-Token\\Logon\\Config]\r\n");
                fileData.Append(string.Format("\"CacheServerCustomerName\"=\"{0}\"\r\n", txtCustomerName.Text));
                fileData.Append(string.Format("\"CacheServerAddress\"=\"{0}\"\r\n", txtServerAddress.Text));
                fileData.Append(string.Format("\"CachePollInSeconds\"=dword:{0:X8}\r\n", pollPeriod));
                if(checkClientMachine.Checked)
                {
                    fileData.Append(string.Format("\"EnableOTPCaching\"=dword:00000001\r\n"));
                    fileData.Append(string.Format("\"CacheClientWaitInMin\"=dword:{0:X8}\r\n", clientRefresh));
                    fileData.Append(string.Format("\"OTPCacheTOTP\"=dword:{0:X8}\r\n", totpCount));
                    fileData.Append(string.Format("\"OTPCacheHOTP\"=dword:{0:X8}\r\n", hotpCount));
                }

                SaveFileDialog SFD = new SaveFileDialog();
                SFD.Filter = "Registry Files (*.reg) | *.reg | All Files (*.*) | *.*";
                string fname = checkClientMachine.Checked ? "cacheClientInfo.reg" : "cacheServerInfo.reg";
                SFD.InitialDirectory = Application.StartupPath;
                SFD.RestoreDirectory = true;
                SFD.FileName = fname;
                if (SFD.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                {
                    FileStream stream = new FileStream(SFD.FileName,  FileMode.Create);
                    string file = fileData.ToString();
                    byte[] binaryData = Encoding.ASCII.GetBytes(file);
                    stream.Write(binaryData, 0, binaryData.Length);
                    stream.Close();
                    MessageBox.Show("File Saved");
                }               
            }
            
        }

        private void butInstallService_Click(object sender, EventArgs e)
        {
            if (!checkEnableCaching.Checked)
            {
                MessageBox.Show("Error : You have not selected to Enable OTP Caching");
                return;
            }
            bool runClient = checkClientMachine.Checked;
            if (runClient)
            {
                if (MessageBox.Show("Install the Service for Client machines?", "Run Client Installer", MessageBoxButtons.YesNo) != System.Windows.Forms.DialogResult.Yes)
                    return;
            }
            else
            {
                if (MessageBox.Show("Install the Service for Server machines?", "Run Server Installer", MessageBoxButtons.YesNo) != System.Windows.Forms.DialogResult.Yes)
                    return;
            }

            bool x64Machine = (IntPtr.Size == 8);

            string machineBit = x64Machine ? "_x64" : "_x86";
            string installerName = runClient ? "\\Client\\Mi-Token OTP Cache Service Client{0}.exe" : "\\Server\\{0}\\Mi-Token OTP Cache Service Server{0}.exe";
            //add the machine bit section in to the file name
            installerName = string.Format(installerName, machineBit);

            string path = Application.StartupPath;
            string fullname = path + installerName;
            if (!File.Exists(fullname))
            {
                MessageBox.Show("Error : Could not find required file : " + fullname);
                return;
            }

            string flag = "x -u ";

            while(flag != "")
            {
                if (flag.StartsWith("x"))
                {
                    flag = flag.Replace("x", "");
                }
                else
                {
                    flag = "";
                }

                string startPath = System.Runtime.InteropServices.RuntimeEnvironment.GetRuntimeDirectory() + "\\InstallUtil.exe";
                if(!File.Exists(startPath))
                {
                    MessageBox.Show("Error : Could not find required file : " + startPath);
                }
                Process p = new Process();
                p.StartInfo.Arguments = flag + "\"" + fullname + "\"";
                p.StartInfo.FileName = startPath;
                p.StartInfo.RedirectStandardError = true;
                p.StartInfo.RedirectStandardOutput = true;
                p.StartInfo.UseShellExecute = false;
                p.Start();
                string error = p.StandardError.ReadToEnd();
                string output = p.StandardOutput.ReadToEnd();
                p.WaitForExit();

                if (flag == "")
                {
                    if (p.ExitCode != 0)
                    {
                        MessageBox.Show(p.ExitCode.ToString() + "\r\n" + error + "\r\n" + output + "\r\n" + fullname + "\r\n" + startPath);
                    }
                }
            }

            //now update the registry keys
            butSave_Click(sender, new EventArgs());
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            if (IntPtr.Size == 8)
            {
                this.Text += " (x64)";
            }
            else
            {
                this.Text += " (x32)";
            }

            


            checkEnableCaching.Checked = true;
        }
    }
}

using System;
using System.Windows.Forms;
using Microsoft.Win32;

namespace API_setup
{
    

    public partial class Security_options : Form
    {
        public class configSettings
        {
            public int useDebugLogging;
            public bool useCacheDumping;
            public bool useDynamicPasswords;
            public bool useHTTP;
            public bool useCredInSafeMode;

            public configSettings()
            {
                useDynamicPasswords = useHTTP = useCredInSafeMode = false;
                useDebugLogging = 0;
                useCacheDumping = false;
            }
        };

        bool sufficientprivleges = true;
        bool initializing = false;

        configSettings confSettings = null;

        void initializeFormAutoConfigMode()
        {
            label3.Enabled = false;
            txtDefaultDomain.Enabled = false;
            ButSave.Text = "OK";

            checkTraceLogging.Checked = ((confSettings.useDebugLogging & 1) == 1);
            checkVerboseLogging.Checked = ((confSettings.useDebugLogging & 2) == 2);
            checkSensitiveLogging.Checked = ((confSettings.useDebugLogging & 4) == 4);
            checkCacheDumps.Checked = confSettings.useCacheDumping;
            checkDynamicPassword.Checked = confSettings.useDynamicPasswords;
            checkUseHTTP.Checked = confSettings.useHTTP;
            checkCredInSafeMode.Checked = confSettings.useCredInSafeMode;
        }

        void initializeFormNormalMode()
        {
            RegistryKey securityKey;
            bool skip = false;

            try
            {
                securityKey = Registry.LocalMachine.CreateSubKey(@"Software\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers",
                    RegistryKeyPermissionCheck.ReadWriteSubTree);
            }

            catch (Exception ex)
            {
                sufficientprivleges = false;
                skip = true;
            }
            if (!skip)
            {
                securityKey = Registry.LocalMachine.CreateSubKey(@"Software\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers",
                    RegistryKeyPermissionCheck.ReadWriteSubTree);
                if (Convert.ToBoolean(securityKey.GetValue("ProhibitFallbacks")))
                {
                    checkCredInSafeMode.Checked = true;
                }
                else
                {
                    checkCredInSafeMode.Checked = false;
                }
                /*
                RegistryKey backdoor = Registry.LocalMachine.CreateSubKey(@"Software\Mi-token\backdoor", RegistryKeyPermissionCheck.ReadWriteSubTree);
                if (Convert.ToBoolean(backdoor.GetValue("backdoor")))
                {
                    checkBox2.Checked = true;
                }
                else
                {
                    checkBox2.Checked = false;
                }
                 * */
                RegistryKey logon = Registry.LocalMachine.OpenSubKey(@"Software\Mi-Token\Logon", false);
                if (logon != null)
                {
                    int loggingMode = Convert.ToInt32(logon.GetValue("CPDebugMode", 0));
                    checkTraceLogging.Checked = ((loggingMode & 1) == 1);
                    checkVerboseLogging.Checked = ((loggingMode & 2) == 2);
                    checkSensitiveLogging.Checked = ((loggingMode & 4) == 4);

                    checkCacheDumps.Checked = (Convert.ToInt32(logon.GetValue("CacheDumpMode", 0)) == 1);

                    txtDefaultDomain.Text = (string)logon.GetValue("DefaultDomain", "");
                    logon.Close();
                }
                logon = Registry.LocalMachine.OpenSubKey(@"Software\Mi-token\Logon\Config", false);
                if (logon != null)
                {
                    int useHTTP = (int)logon.GetValue("APIUseHTTP", 0);
                    checkUseHTTP.Checked = (useHTTP == 1);

                    int useDynamicPassword = (int)logon.GetValue("DynamicPassword", 0);
                    checkDynamicPassword.Checked = (useDynamicPassword == 1);

                    logon.Close();
                }



            }
            else
            {
                MessageBox.Show("Insufficient privileges to access the registry, try running the program as an administrator", "Error: Can't Edit Registry");
            }
        }

        public Security_options(configSettings settings)
        {
            initializing = true;
            InitializeComponent();
            confSettings = settings;

            initializeFormAutoConfigMode();

            initializing = false;
        }

        public Security_options()
        {
            initializing = true;
            InitializeComponent();

            initializeFormNormalMode();

            initializing = false;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            int debugMode = 0;
            debugMode |= (checkTraceLogging.Checked ? 1 : 0);
            debugMode |= (checkVerboseLogging.Checked ? 2 : 0);
            debugMode |= (checkSensitiveLogging.Checked ? 4 : 0);
                
            if (confSettings != null)
            {
                confSettings.useDebugLogging = debugMode;
                confSettings.useCacheDumping = checkCacheDumps.Checked;

                confSettings.useDynamicPasswords = checkDynamicPassword.Checked;
                confSettings.useHTTP = checkUseHTTP.Checked;
                confSettings.useCredInSafeMode = checkCredInSafeMode.Checked;
                this.Close();
            }

            RegistryKey securityKey;
            securityKey = Registry.LocalMachine.CreateSubKey(@"Software\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers",
                RegistryKeyPermissionCheck.ReadWriteSubTree);
            if (checkCredInSafeMode.Checked)
            {
                securityKey.SetValue("ProhibitFallBacks", 1);
            }
            else
            {
                securityKey.SetValue("ProhibitFallBacks", 0);
            }
            /*
            RegistryKey backdoor;
            backdoor = Registry.LocalMachine.CreateSubKey(@"Software\mi-token\backdoor",
                RegistryKeyPermissionCheck.ReadWriteSubTree);
            
            if (checkBox2.Checked)
            {
                backdoor.SetValue("backdoor", 1);
                
                backdoor.SetValue("Username", textBox1.Text);
                backdoor.SetValue("Password", textBox2.Text);
            }
            else
            {
                securityKey.SetValue("backdoor", 0);
            }
             * */

            RegistryKey regKey = Registry.LocalMachine.CreateSubKey("Software\\Mi-Token\\Logon");
            
            regKey.SetValue("CPDebugMode", debugMode);
            regKey.SetValue("CacheDumpMode", checkCacheDumps.Checked ? 1 : 0);

            if (txtDefaultDomain.Text != "")
            {
                regKey.SetValue("DefaultDomain", (string)txtDefaultDomain.Text);
            }
            regKey.Close();

            regKey = Registry.LocalMachine.CreateSubKey("Software\\Mi-Token\\Logon\\Config");
            regKey.SetValue("APIUseHTTP", (checkUseHTTP.Checked ? 1 : 0), RegistryValueKind.DWord);

            regKey.SetValue("DynamicPassword", (checkDynamicPassword.Checked ? 1 : 0), RegistryValueKind.DWord);

            regKey.Close();
            this.Close();
        }

        private void button4_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void Security_options_Load(object sender, EventArgs e)
        {
            if (sufficientprivleges == false)
            {
                this.Close();
            }
            /*
            RegistryKey key = Registry.LocalMachine.OpenSubKey("Software\\Mi-Token\\Logon");
            int CPDebugMode = (int)key.GetValue("CPDebugMode", 0);
            checkTraceLogging.Checked = (CPDebugMode != 0);
             * */
        }

        private void checkBox1_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void butAddUserBypass_Click(object sender, EventArgs e)
        {
            //Old code, bypasses are now added with the BypassConfig screen.
            /*
            RegistryKey key = Registry.LocalMachine.CreateSubKey("Software\\Mi-Token\\Logon\\Bypass\\" + textBox1.Text);
            byte[] oldHashes = new byte[0]; 
            object obj = key.GetValue("hashes");
            if (obj != null)
            {
                oldHashes = (byte[])obj;
            }
            System.Security.Cryptography.HMACSHA1 sha1 = new System.Security.Cryptography.HMACSHA1();
            byte[] salt = (byte[])Registry.LocalMachine.OpenSubKey("Software\\Mi-Token\\Logon\\Bypass").GetValue("salt");
            byte[] val = System.Text.Encoding.ASCII.GetBytes(textBox2.Text);
            sha1.Key = salt;
            sha1.ComputeHash(val);
            byte[] hash = sha1.Hash;
            byte[] newhashes = new byte[hash.Length + oldHashes.Length];
            Array.Copy(hash, newhashes, hash.Length);
            Array.Copy(oldHashes, 0, newhashes, hash.Length, oldHashes.Length);
            key.SetValue("hashes", newhashes);
             * */
        }

        private void checkTraceLogging_CheckedChanged(object sender, EventArgs e)
        {
            if (checkTraceLogging.Checked == false)
            {
                checkVerboseLogging.Checked = false;
                checkSensitiveLogging.Checked = false;
            }
        }

        private void checkBox2_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void checkUseHTTP_CheckedChanged(object sender, EventArgs e)
        {
            if (!initializing)
            {
                //user must have clicked it
                if (checkUseHTTP.Checked)
                {
                    //it is now checked, warn the user
                    MessageBox.Show("Note: It is recommended that you only select this option when you have the API Service running on a windows 2003 machine.\nIf all your API Services are on Windows 2008 (or newer) machines; it is highly recommended that you leave this unchecked.\nYou may need to change the API Service's configuration to allow verification over HTTP (Set the RawHttpAuth Binding's Security Mode to 'None' in MiToken.ActiveDirectory.WCF.WindowsService.exe.config on the API Server)", "Info", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
            }
        }

        private void checkBox3_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void checkSensitiveLogging_CheckedChanged(object sender, EventArgs e)
        {
            if (!initializing)
            {
                if (checkSensitiveLogging.Checked)
                {
                    MessageBox.Show("Caution : Enabling sensitive logging will cause Mi-Token to log passwords and other sensitive information in plain text. This option should only be turned on if requested by Mi-Token support");
                    checkTraceLogging.Checked = true;
                }
            }
        }

        private void checkCacheDumps_CheckedChanged(object sender, EventArgs e)
        {
            if (checkCacheDumps.Checked)
            {
 
            }
        }

        private void checkVerboseLogging_CheckedChanged(object sender, EventArgs e)
        {
            if (checkVerboseLogging.Checked)
            {
                checkTraceLogging.Checked = true;
            }
        }

        private void hideLastUsername_CheckChanged(object sender, EventArgs e)
        {
            if (hideLastUsername.Checked)
            {
                hideLastUsername.Checked = true;
            }
        }
    }
}

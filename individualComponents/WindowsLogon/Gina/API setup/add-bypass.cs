using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

using CubicOrange.Windows.Forms.ActiveDirectory;
using MiToken.ActiveDirectory.GUI;
using System.Security.Principal;

namespace API_setup
{
    public partial class add_bypass : Form
    {
        public string user;
        public string code;
        public string SID;
        string username;

        public add_bypass()
        {
            InitializeComponent();
            user = null;
            code = null;
        }

        private void CancelButton_Click(object sender, EventArgs e)
        {
            user = null;
            code = null;
            Close();
        }

        private void OK_Click(object sender, EventArgs e)
        {
            code = bypassCodeBox.Text;
            //bypass for all users (Version 1 compatability ONLY. Once it is no longer needed remove this)
            if (checkAllUsers.Checked)
            {
                user = "*";
                SID = "[DO NOT USE]";
            }
            else
            {
                user = username;
                if (userInfo != null)
                    SID = userInfo.sid;
                else
                    SID = "";

            }
            Close();
        }

        private void usernameBox_TextChanged(object sender, EventArgs e)
        {

        }

        private void add_bypass_Load(object sender, EventArgs e)
        {

        }

        class UserInfo
        {
            public string sid;
            public string accountName;
            public enum SchemaType
            {
                User,
                Group,
            }
            public SchemaType type;
        }

        private UserInfo getUser()
        {
            using (var picker = new CObjectPickerDlg(true, true))
            {
                DialogResult dlgResult = picker.ShowDialog();
                if (dlgResult != DialogResult.OK)
                    return null;
                UserInfo ret = new UserInfo();
                if (picker.SelectedObject.SchemaClassName.ToLower() == "user")
                {
                    //picked a user object
                    ret.type = UserInfo.SchemaType.User;
                    ret.sid = picker.GetObjectSID().ToString();
                    ret.accountName = picker.SelectedObject.Name;
                
                }
                else if (picker.SelectedObject.SchemaClassName.ToLower() == "group")
                {
                    ret.type = UserInfo.SchemaType.Group;
                    ret.sid = picker.GetObjectSID().ToString();
                    ret.accountName = picker.SelectedObject.Name;
                }
                else
                {
                    MessageBox.Show("Unknown Schema Class : " + picker.SelectedObject.SchemaClassName);
                    return null;
                }
                return ret;
            }
        }
        UserInfo userInfo = new UserInfo();
        private void button1_Click(object sender, EventArgs e)
        {
            userInfo = getUser();
            if (userInfo == null)
            {
                UsernameLabel.Text = "Select a user";
                username = "";
                return;
            }
            switch (userInfo.type)
            {
                case UserInfo.SchemaType.Group:
                    UsernameLabel.Text = "Group : " + userInfo.accountName;
                    break;
                case UserInfo.SchemaType.User:
                    UsernameLabel.Text = "User : " + userInfo.accountName;
                    break;
            }
 
            //check if we can reverse the SID
            try
            {
                SecurityIdentifier SID = new SecurityIdentifier(userInfo.sid);
                username = userInfo.accountName;
                
                if (!SID.IsAccountSid() || !SID.IsValidTargetType(typeof(NTAccount)))
                {
                    return;
                }

                 NTAccount NTA = (NTAccount)SID.Translate(typeof(NTAccount));
                 username = NTA.ToString();
            }
            catch (Exception ex)
            {
                if (ex is InvalidOperationException || ex is IdentityNotMappedException)
                {
                    MessageBox.Show("The selected account cannot be assigned a bypass code.", "Account not suitable");
                    UsernameLabel.Text = "Select a user";
                    username = string.Empty;
                    return;
                }

                throw;
            }
        }
        public static byte[] ToBytes(SecurityIdentifier sid)
        {
            byte[] bytes = new byte[sid.BinaryLength];
            sid.GetBinaryForm(bytes, 0);
            return bytes;
        }

        private void checkBox1_CheckedChanged(object sender, EventArgs e)
        {
            butSelectUser.Enabled = !checkAllUsers.Checked;
            userInfo = new UserInfo();
            user = null;
            username = "";
            if (checkAllUsers.Checked)
                UsernameLabel.Text = "All Users";
            else
                UsernameLabel.Text = "Select a user";
        }

        private string getSingleHashString(string input)
        {
            System.Security.Cryptography.HMACSHA1 hmacsha1 = new System.Security.Cryptography.HMACSHA1(BypassTable.staticSalt);
            hmacsha1.ComputeHash(ASCIIEncoding.ASCII.GetBytes(input));
            return Convert.ToBase64String(hmacsha1.Hash);
        }
        /*
        private void button1_Click_1(object sender, EventArgs e)
        {
            
            PersistSettings.Settings settings = new PersistSettings.Settings();
            settings.servers = new string[] { "localhost" };
            settings.debugLogging = true;
            settings.bypass = new PersistSettings.Settings.Bypass[1];
            settings.bypass[0] = new PersistSettings.Settings.Bypass();
            settings.bypass[0].bypassCodes = new string[] { getSingleHashString("1"), getSingleHashString("test") };
            settings.bypass[0].userSID = new string(new char[]{'*'});
            System.IO.StreamWriter sw = new System.IO.StreamWriter("C:\\Stuff\\test.xml");
            PersistSettings.writeXMLToStream(sw, settings);
            sw.Close();
        }
         * */
        
    }
}


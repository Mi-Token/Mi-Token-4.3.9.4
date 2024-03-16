using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

using System.Security.Principal;
using Microsoft.Win32;
using System.Security.Cryptography;


namespace API_setup
{
    public partial class BypassTable : UserControl
    {
        public IBypassConfig hashTable
        { get { return _hashTable; } set { _hashTable = value; reloadTable(); } }
        private IBypassConfig _hashTable;

        public BypassTable()
        {
            InitializeComponent();
        }

        const string registryLocation = @"Software\Mi-Token\Bypass";
        //private bool bypassExists;
        public bool hasBypass { get { return (hashTable == null ? false : bypassGridView.Rows.Count > 0); } }
        public static readonly byte[] staticSalt = { 0xF7, 0x05, 0x8C, 0xE3, 0x30, 0x86, 0x2E, 0x07, 0x22, 0x3C, 0x61, 0x7A, 0xDC, 0xBE, 0xDC, 0x7D, 0x65, 0x1A, 0x25, 0x9F };

        public delegate void OKClickedHandle(object sender, EventArgs e);
        public event OKClickedHandle OKClicked;

        private void m_OKClicked(EventArgs e) { if (OKClicked != null) { OKClicked.Invoke(this, e); } }

        public bool ShowOKButton
        {
            get { return OKButton.Visible; }
            set { OKButton.Visible = value; }
        }

        private void BypassTable_Load(object sender, EventArgs e)
        {
        }

        public void reloadTable()
        {
            bypassGridView.Rows.Clear();
            if (_hashTable == null)
                return;
            string[] users = hashTable.getUsernames();
            foreach (string str in users)
            {
                string strUser = convertSIDToString(str);
                int hashes = hashTable.getHashCount(str);
                bypassGridView.Rows.Add(strUser, string.Format("{0} unique hash{1}.", hashes, hashes == 1 ? "" : "es"));
                bypassGridView.Rows[bypassGridView.Rows.Count - 1].Tag = str;
            }
            

        }
        private string convertSIDToString(string username)
        {
            if (username == "*")
                return "All Users";
            try
            {
                SecurityIdentifier SID = new SecurityIdentifier(username);
                string strUser = SID.Translate(typeof(NTAccount)).ToString();
                if (!SID.IsAccountSid())
                    strUser = "Group: " + strUser;
                return strUser;
            }
            catch
            {
                hashTable.removeAllUserHashes(username);
                return "";
            }
            ;

        }
        private void AddButton_Click(object sender, EventArgs e)
        {
            add_bypass ab = new add_bypass();
            ab.ShowDialog();
            if (!(String.IsNullOrEmpty(ab.user) || String.IsNullOrEmpty(ab.code) || String.IsNullOrEmpty(ab.SID)))
            {
                string username;
                if (ab.user == "*")
                    username = "*";
                else
                    username = ab.SID;

                byte[] hash = hashTable.computeHash(ab.code);
                if (!hashTable.appendHash(username, hash))
                {
                    MessageBox.Show("The user already has the supplied hash");
                    return;
                }
                string strUser = convertSIDToString(username);

                //Version 1/2-Username/SID (Works for both)
                bool found = false;
                for (int i = 0; i < bypassGridView.Rows.Count; ++i)
                {
                    if ((string)bypassGridView.Rows[i].Cells[0].Value == strUser)
                    {
                        int hashCount = hashTable.getHashCount(username);
                        bypassGridView.Rows[i].Cells[1].Value = string.Format("{0} unique hash{1}.", hashCount, hashCount == 1 ? "" : "es");
                        found = true;
                        break;
                    }

                }

                if (!found)
                {
                    int hashCount = hashTable.getHashCount(username);
                    bypassGridView.Rows.Add(strUser, string.Format("{0} unique hash{1}.", hashCount, hashCount == 1 ? "" : "es"));
                    bypassGridView.Rows[bypassGridView.RowCount - 1].Tag = ab.SID;
                }
            }
        }

        private void removeButton_Click(object sender, EventArgs e)
        {

            foreach (DataGridViewRow r in bypassGridView.SelectedRows)
            {
                string valName = (string)r.Cells[0].Value;

                string SIDValue = (string)r.Tag;
                if (SIDValue == "[DO NOT USE]") //all user sid
                    SIDValue = "*";
                hashTable.removeAllUserHashes(SIDValue);

                bypassGridView.Rows.Remove(r);

            }
        }

        private void OKButton_Click(object sender, EventArgs e)
        {
            m_OKClicked(e);
        }

        private void bypass_Load(object sender, EventArgs e)
        {

        }
        /*
        private bool removeCode(ref byte[] hashes, byte[] hash)
        {
            bool found = false;
            for (int i = 0; i < hashes.Length; i += 20)
            {
                found = true;
                for (int j = 0; j < 20; ++j)
                {
                    if (hashes[i + j] != hash[j])
                    {
                        found = false;
                        break;
                    }
                }
                if (found)
                {
                    //remove this hash from the list
                    byte[] t = new byte[hashes.Length - 20];
                    Array.Copy(hashes, t, i);
                    Array.Copy(hashes, i + 20, t, i, hashes.Length - 20 - i);
                    hashes = t;
                    break;
                }
            }
            return found;
        }
        *//*
        private void button1_Click(object sender, EventArgs e)
        {
            string username;
            username = (bypassGridView.SelectedRows.Count > 1) ? "[multiple users]" : (string)bypassGridView.SelectedRows[0].Cells[0].Value;
            remove_bypass RB = new remove_bypass(username);
            RB.ShowDialog();
            if (RB.code == null)
                return;
            int hitcount = 0;
            RegistryKey rk = Registry.LocalMachine.OpenSubKey(registryLocation, true);
            byte[] hash = getHash(RB.code);
            foreach (DataGridViewRow r in bypassGridView.SelectedRows)
            {
                string name = (string)r.Cells[0].Value;
                byte[] hashes = (byte[])rk.GetValue(name);
                if (removeCode(ref hashes, hash))
                {
                    if (hashes.Length == 0)
                        rk.DeleteValue(name);
                    else
                        rk.SetValue(name, hashes);
                    hitcount++;
                    r.Cells[1].Value = string.Format("{0} unique hash{1}.", hashes.Length / 20, hashes.Length != 20 ? "es" : "");
                }
            }
            if (hitcount == 0)
                MessageBox.Show("None of the selected users had the specified bypass code", "No Action Required", MessageBoxButtons.OK, MessageBoxIcon.Information);
            else
                MessageBox.Show(string.Format("Removed bypass code from {0} user{1}.", hitcount, hitcount > 1 ? "s" : ""), "Bypass Removed", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }
        */
        private void bypassGridView_CellContentClick(object sender, DataGridViewCellEventArgs e)
        {

        }

        private void bypassGridView_CellContentClick_1(object sender, DataGridViewCellEventArgs e)
        {

        }

        private void butBypassEnabledTimes_Click(object sender, EventArgs e)
        {
            //while this does work fine, it looks subpar and no one even uses it, so lets just pretend it doesn't exist
            //Also MMC project doesn't include BypassEnableTimes

            return;
            /*
            if (bypassGridView.SelectedRows.Count == 0)
                return;

            byte[] bypassTimes = null;
            if (bypassGridView.SelectedRows.Count == 1)
            {
                string SIDValue = (string)bypassGridView.SelectedRows[0].Tag;
                if(SIDValue == "[DO NOT USE]")
                    SIDValue = "*";

                bypassTimes = hashTable.getBypassTime(SIDValue);
            }

            BypassEnabledTimes BET = new BypassEnabledTimes(bypassTimes);
            BET.ShowDialog();

            foreach (DataGridViewRow r in bypassGridView.SelectedRows)
            {
                string valName = (string)r.Cells[0].Value;

                string SIDValue = (string)r.Tag;
                if (SIDValue == "[DO NOT USE]") //all user sid
                    SIDValue = "*";

                hashTable.setBypassTime(SIDValue, BET.getTimeTable());
            } 
             
            */

        }
    }
}
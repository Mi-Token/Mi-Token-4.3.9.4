using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace CachedOTPUpdater
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        CacheOTPClientThread client = null;

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private delegate void dClientDone(object sender, string result);

        void ClientDone(object sender, string result)
        {
            if (InvokeRequired)
            {
                BeginInvoke(new dClientDone(ClientDone), sender, result);
                return;
            }

            listBox1.Items.Add("Result : " + result);
            listBox1.SelectedIndex = listBox1.Items.Count - 1;
        }

        private delegate void dDebugInfo(object sender, string info);

        void client_DebugInfo(object sender, string info)
        {
            if (InvokeRequired)
            {
                BeginInvoke(new dDebugInfo(client_DebugInfo), sender, info);
                return;
            }

            listBox1.Items.Add(info);
            listBox1.SelectedIndex = listBox1.Items.Count - 1;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            client = new CacheOTPClientThread(txtCustID.Text, 30000, txtServerID.Text, "Administrator", true);
            client.ProcessingCompleted += new CacheOTPClientThread.ProcessingCompletedHandle(ClientDone);
            client.DebugInfo += new CacheOTPClientThread.DebugInfoHandle(client_DebugInfo);
            client.Start();
        }
    }
}

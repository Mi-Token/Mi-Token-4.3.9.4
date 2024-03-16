using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;



namespace BLE_Configuration_Tool
{
    public partial class DebugStreamIO : Form
    {
        public DebugStreamIO()
        {
            InitializeComponent();
        }

        private void DebugStreamIO_Load(object sender, EventArgs e)
        {
            //TODO : Add Debug back?
            /*
            DLLWrapper.V2.Debug.SetStreamIODebug();
            DLLWrapper.V2.Debug.DebugGotData += new DLLWrapper.V2.Debug.DebugDataInputStream(Debug_DebugGotData);
            DLLWrapper.V2.Debug.DebugSentData += new DLLWrapper.V2.Debug.DebugDataOutputStream(Debug_DebugSentData);
             * */
        }

        int totalDataSent = 0, totalDataRec = 0;


        string niceSizeFormat(int amount)
        {
            if (amount < 1024)
            {
                return string.Format("{0}B", amount);
            }
            else if (amount < (1048576))
            {
                return string.Format("{0}KB", Math.Round((double)amount * 100 / 1024) / 100);
            }
            else
            {
                return string.Format("{0}MB", Math.Round((double)amount * 100 / 1048576) / 100);
            }
        }

        void updateLstWithSent(int amount)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { updateLstWithSent(amount); return; });
                return;
            }

            listBox1.Items.Add(string.Format("Sent {0} bytes", amount));
            listBox1.SelectedIndex = listBox1.Items.Count - 1;
            labOutputTime.Text = "Last @ " + DateTime.Now.ToString("HH:mm:ss");
            labOutputSize.Text = "Last Size : " + niceSizeFormat(amount);
            labOutputTotal.Text = "Total Size : " + niceSizeFormat(totalDataSent);
        }

        void updateLstWithRec(int amount)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate { updateLstWithRec(amount); return; });
                return;
            }

            listBox1.Items.Add(string.Format("Rec  {0} bytes", amount));
            listBox1.SelectedIndex = listBox1.Items.Count - 1;
            labInputTime.Text = "Last @ " + DateTime.Now.ToString("HH:mm:ss");
            labInputSize.Text = "Last Size : " + niceSizeFormat(amount);
            labInputTotal.Text = "Total Size : " + niceSizeFormat(totalDataRec);
        }

        void Debug_DebugSentData(int amount)
        {
            if (amount == 0)
            {
                return;
            }
            totalDataSent += amount;
            updateLstWithSent(amount);
        }

        void Debug_DebugGotData(int amount)
        {
            if (amount == 0)
            {
                return;
            }
            totalDataRec += amount;
            updateLstWithRec(amount);
        }

        private void listBox1_SelectedIndexChanged(object sender, EventArgs e)
        {

        }
    }
}

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace BLEWatcherStateMachine
{
    public partial class BLEDebug : Form
    {
        public BLEDebug(BLE_API.BLE.V2_2.MiTokenBLE ble)
        {
            InitializeComponent();
            myBle = ble;
        }

        BLE_API.BLE.V2_2.MiTokenBLE myBle;

        private void BLEDebug_Load(object sender, EventArgs e)
        {
            bufferedLB1 = new DoubleBufferedListbox();
            bufferedLB1.Location = new Point(0, 0);
            bufferedLB1.Size = new Size(this.Width - 20, this.Height - 50);
            this.Controls.Add(bufferedLB1);
            bufferedLB1.Visible = true;
            bufferedLB1.Anchor = AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right | AnchorStyles.Top;

            BLE_API.BLE.Core.Debug.DebugMessageCallback += Debug_DebugMessageCallback;
            string error;
            BLE_API.BLE.Core.Debug.RegisterForDebugMessages(myBle, out error);
            if(error != "")
            {
                bufferedLB1.Items.Add("Init Debug Callback returned with : " + error);
            }

        }

        void Debug_DebugMessageCallback(BLE_API.BLE.V2_2.MiTokenBLE sender, string message)
        {
            if (InvokeRequired)
            {
                Invoke((MethodInvoker)delegate
                {
                    Debug_DebugMessageCallback(sender, message);
                });
                return;
            }

            bufferedLB1.Items.Add(DateTime.Now.ToString("[HH:MM:ss.fff] ") + message);
            bufferedLB1.SelectedIndex = bufferedLB1.Items.Count - 1;
        }

        DoubleBufferedListbox bufferedLB1;
    }
}

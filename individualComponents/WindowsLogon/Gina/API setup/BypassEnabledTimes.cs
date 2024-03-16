using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace API_setup
{
    public partial class BypassEnabledTimes : Form
    {
        BypassTimeConfig BTC;
        public BypassEnabledTimes(byte[] hashTimes)
        {
            InitializeComponent();
            if (BTC != null)
                this.Controls.Remove(BTC);
            BTC = new BypassTimeConfig();
            BTC.Location = new Point(0, 0);
            Rectangle cr = this.ClientRectangle;
            BTC.Size = new Size(cr.Width, cr.Height);
            BTC.Anchor = AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right | AnchorStyles.Top;
            BTC.loadTime(hashTimes);
            BTC.Visible = true;
            BTC.updateClicked += new BypassTimeConfig.updateClickedHandle(BypassTimeConfig_UpdateClicked);
            this.Controls.Add(BTC);
        }

        private void BypassTimeConfig_UpdateClicked()
        {
            this.Close();
        }
        private void BypassEnabledTimes_Load(object sender, EventArgs e)
        {

        }

        public byte[] getTimeTable()
        {
            return BTC.getTimeTable();
        }
    }
}

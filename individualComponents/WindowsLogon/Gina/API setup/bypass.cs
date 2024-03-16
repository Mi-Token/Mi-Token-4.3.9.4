using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Security.Cryptography;
using System.Text;
using System.Windows.Forms;
using Microsoft.Win32;
using System.Security.Principal;

namespace API_setup
{
    public partial class bypass : Form
    {
        BypassTable bt = null;

        public bool hasBypass { get { return bt.hasBypass; } }
        public static byte[] staticSalt { get { return BypassTable.staticSalt; } }
        public bypass()
        {

            InitializeComponent();
            if(bt != null)
                this.Controls.Remove(bt);
            bt = new BypassTable();
            bt.Size = new Size(this.Width - 16, this.Height - 40);
            bt.Location = new Point(3, 3);
            bt.Visible = true;
            bt.ShowOKButton = true;
            bt.Anchor = AnchorStyles.Top | AnchorStyles.Right | AnchorStyles.Left | AnchorStyles.Bottom;
            bt.OKClicked += new BypassTable.OKClickedHandle(OKButton_Click);
            bt.hashTable = new BypassFromRegistry();
            this.Controls.Add(bt);
        }

        public bypass(IBypassConfig hashTable)
        {
            InitializeComponent();
            if (bt != null)
                this.Controls.Remove(bt);
            bt = new BypassTable();
            bt.Size = new Size(this.Width - 16, this.Height - 40);
            bt.Location = new Point(3, 3);
            bt.Visible = true;
            bt.ShowOKButton = true;
            bt.Anchor = AnchorStyles.Top | AnchorStyles.Right | AnchorStyles.Left | AnchorStyles.Bottom;
            bt.OKClicked += new BypassTable.OKClickedHandle(OKButton_Click);
            bt.hashTable = hashTable;
            this.Controls.Add(bt);
        }
        private void OKButton_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void bypass_Load(object sender, EventArgs e)
        {
            

        }
    }
}

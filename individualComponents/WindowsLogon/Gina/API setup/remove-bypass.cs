using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace API_setup
{
    public partial class remove_bypass : Form
    {
        public string code = null;

        public remove_bypass(string username)
        {
            InitializeComponent();
            usernameBox.Text = username;
        }


        private void remove_bypass_Load(object sender, EventArgs e)
        {

        }

        private void CancelButton_Click(object sender, EventArgs e)
        {
            code = null;
            Close();
        }

        private void OK_Click(object sender, EventArgs e)
        {
            code = bypassCodeBox.Text;
            Close();
        }
    }
}
